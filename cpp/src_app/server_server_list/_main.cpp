#include "../lib_server_util/all.hpp"
#include "./server_list_manager.hpp"

#include <pp_common/base.hpp>
#include <pp_protocol/command.hpp>
#include <pp_protocol/internal/all.hpp>

static auto RegisterServiceBindAddress = xNetAddress();
static auto DownloadServiceBindAddress = xNetAddress();

auto IC  = xIoContext();
auto ICG = xResourceGuard(IC);

struct xRegisterServerService : xService {

    void OnClientConnected(xServiceClientConnection & Connection) override { Logger->I("OnClientConnected"); }

    void OnClientClose(xServiceClientConnection & Connection) override {
        Logger->I("OnClientClose");
        auto P = static_cast<xSL_AuthCacheServerInfo *>(Connection.GetUserContext().P);
        if (!P) {
            return;
        }
        auto SP       = P->ServerType;
        auto ServerId = P->ServerId;
        switch (SP) {
            case eServerType::AUTH_CACHE:
                Logger->I("RemoveAuthCacheServerInfo: ServerId=%" PRIx64 "", ServerId);
                ServerListManager.RemoveAuthCacheServerInfo(ServerId);
                break;
            default:
                break;
        };
        return;
    }

    bool OnClientPacket(xServiceClientConnection & Connection, xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) override {

        Logger->I("OnClientPacket CommandId=%" PRIx32 "", CommandId);
        switch (CommandId) {
            case Cmd_RegisterAuthCacheServer:
                return OnRegisterAuthCacheServer(Connection, CommandId, RequestId, PayloadPtr, PayloadSize);
            case Cmd_RegisterDeviceAuditServer:
                return true;
            case Cmd_RegisterAccountAuditCollectorServer:
                return true;
            default:
                break;
        }
        return true;
    }

    bool OnRegisterAuthCacheServer(xServiceClientConnection & Connection, xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) {
        auto & P = Connection.GetUserContext().P;
        if (P) {
            Logger->D("duplicated register server");
            return false;
        }
        auto R = xPP_RegisterAuthCacheServer();
        if (!R.Deserialize(PayloadPtr, PayloadSize)) {
            Logger->E("invalid request");
            return false;
        }
        auto NP = ServerListManager.AddAuthCacheServerInfo(R.ServerId, R.ExportServerAddress);
        if (!NP) {
            Logger->E("failed to allocate auth cache server info");
            return false;
        }
        P = NP;
        Logger->I("OnRegisterAuthCacheServer: ServerId=%" PRIx64 "", R.ServerId);
        return true;
    }

    bool OnRegisterDeviceAuditServer(xServiceClientConnection & Connection, xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) {
        return false;
    }

    bool OnRegisterAccountAuditCollectorServer(
        xServiceClientConnection & Connection, xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize
    ) {
        return false;
    }

    //
private:
    xSL_InternalServerListManager ServerListManager;
};

xRegisterServerService RegisterServerService;

struct xDownloadServerService : xService {

    void OnClientConnected(xServiceClientConnection & Connection) override {}

    void OnClientClose(xServiceClientConnection & Connection) override {}
    bool OnClientPacket(xServiceClientConnection & Connection, xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) override {
        switch (CommandId) {
            case Cmd_DownloadAuthCacheServerList:
                return OnDownloadAuthCacheServerList(Connection, CommandId, RequestId, PayloadPtr, PayloadSize);
            case Cmd_DownloadDeviceAuditServerList:
                return true;
            case Cmd_DownloadAccountAuditCollectorServerList:
                return true;
            default:
                break;
        }

        return false;
    }

    //
    bool OnDownloadAuthCacheServerList(xServiceClientConnection & Connection, xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) {
        return false;
    }
};

xDownloadServerService DownloadServerService;

int main(int argc, char ** argv) {
    auto SEG = xRuntimeEnvGuard(argc, argv);
    auto CL  = xConfigLoader(RuntimeEnv.DefaultConfigFilePath);
    CL.Require(RegisterServiceBindAddress, "RegisterServiceBindAddress");
    CL.Require(DownloadServiceBindAddress, "DownloadServiceBindAddress");

    auto RSSG = xResourceGuard(RegisterServerService, &IC, RegisterServiceBindAddress, 5000, true);
    auto DSSG = xResourceGuard(DownloadServerService, &IC, DownloadServiceBindAddress, 5000, true);

    while (true) {
        ServiceTicker.Update();
        IC.LoopOnce();
        TickAll(ServiceTicker(), RegisterServerService, DownloadServerService);
    }

    return 0;
}
