#include "../lib_server_util/extra/account_audit_server_list_client.hpp"
#include "../lib_server_util/extra/auth_cache_server_list_client.hpp"
#include "../lib_server_util/extra/device_audit_server_list_client.hpp"
#include "./_global.hpp"

auto IC  = xIoContext();
auto ICG = xResourceGuard(IC);

struct D_AC : xDownloadAuthCacheServerListClient {
    void OnServerListUpdated() {
        for (auto & SI : SortedServerInfoList) {
            Logger->I("ServerId: %" PRIx64 ", ServerAddress: %s", SI.ServerId, SI.Address.ToString().c_str());
        }
        Logger->I("auth cache server list version updated: %" PRIu32 "", ServerListVersion);
    }
} D_AC_Client;

struct D_AD : xDownloadAuditDeviceServerListClient {
    void OnServerListUpdated() {
        for (auto & SI : SortedServerInfoList) {
            Logger->I("ServerId: %" PRIx64 ", ServerAddress: %s", SI.ServerId, SI.Address.ToString().c_str());
        }
        Logger->I("audit device server list version updated: %" PRIu32 "", ServerListVersion);
    }
} D_AD_Client;

struct D_AA : xDownloadAuditAccountServerListClient {
    void OnServerListUpdated() {
        for (auto & SI : SortedServerInfoList) {
            Logger->I("ServerId: %" PRIx64 ", ServerAddress: %s", SI.ServerId, SI.Address.ToString().c_str());
        }
        Logger->I("audit account server list version updated: %" PRIu32 "", ServerListVersion);
    }
} D_AA_Client;

int main(int argc, char ** argv) {
    auto REG = xRuntimeEnvGuard(argc, argv);
    LoadConfig();

    auto AC_CG = xResourceGuard(D_AC_Client, &IC, ServerListServerAddress);
    RuntimeAssert(AC_CG);

    auto DA_CG = xResourceGuard(D_AD_Client, &IC, ServerListServerAddress);
    RuntimeAssert(DA_CG);

    auto AA_CG = xResourceGuard(D_AA_Client, &IC, ServerListServerAddress);
    RuntimeAssert(AA_CG);

    while (true) {
        ServiceTicker.Update();
        IC.LoopOnce();
        TickAll(ServiceTicker(), D_AC_Client, D_AD_Client, D_AA_Client);
    }

    return 0;
    //
}
