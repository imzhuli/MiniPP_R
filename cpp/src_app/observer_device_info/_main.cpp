#include "../lib_server_util/all.hpp"
#include "./_global.hpp"

#include <functional>
#include <pp_common/_.hpp>
#include <pp_protocol/ad_bk/device_info.hpp>
#include <pp_protocol/command.hpp>
#include <pp_protocol/internal/device_state.hpp>
#include <unordered_set>

static auto IC                = xIoContext();
static auto ICG               = xResourceGuard(IC);
static auto GlobalTicker      = xTicker();
static auto DispatcherAddress = xNetAddress();

static auto KR               = xKfkProducer();
static auto SecurityProtocol = std::string();
static auto SaslMechanism    = std::string();
static auto SaslUsername     = std::string();
static auto SaslPassword     = std::string();
static auto BootstrapServers = std::string();
static auto Topic            = std::string();

struct xODI_DeviceKeepAliveNode : xListNode {};
using xODI_DeviceKeepAliveList = xList<xODI_DeviceKeepAliveNode>;

struct xODI_DeviceInfo : xODI_DeviceKeepAliveNode {

    std::string DeviceUuid;
    uint64_t    OnlineTimestampMS        = 0;
    uint64_t    LastKeepAliveTimestampMS = 0;

    uint32_t    Version;
    xNetAddress PrimaryIpv4Address;
    xNetAddress PrimaryIpv6Address;
    uint64_t    TotalUploadSizeSinceOnline;    // 指发向目标的数据
    uint64_t    TotalDownloadSizeSinceOnline;  // 下载数据
    uint32_t    CurrentConnectionCount;
    uint32_t    CurrentUdpChannelCount;

    bool SupportUdpChannel;
    bool SupportDnsRequests;
    bool SpeedLimitEnabled;

    uint32_t TotalNewConnectionsSinceLastPost;
    uint32_t TotalClosedConnectionSinceLastPost;
    uint32_t TotalNewUdpChannelSinceLastPost;
    uint32_t TotalClosedUdpChannelSinceLastPost;
    uint32_t TotalDnsRequestSinceLastPost;
};

static auto Ticker            = xTicker();
static auto DeviceMap         = std::unordered_map<std::string, xODI_DeviceInfo>();
static auto DeviceTimeoutList = xODI_DeviceKeepAliveList();

struct xODI : public xClientPool {
    using xClientPool::Clean;
    using xClientPool::Init;

    void OnDeviceUpdate(ubyte * PayloadPtr, size_t PayloadSize) {
        auto PP = xPP_DeviceInfoUpdate();
        if (!PP.Deserialize(PayloadPtr, PayloadSize)) {
            X_DEBUG_PRINTF("Invalid device info");
            return;
        }

        if (PP.IsOffline) {
            auto Iter = DeviceMap.find(PP.DeviceUuid);
            if (Iter == DeviceMap.end()) {
                X_DEBUG_PRINTF("Found offline device");
                return;
            }
            PostDeviceInfo(&Iter->second, false);
            DeviceMap.erase(Iter);
            return;
        }

        auto   NowMS = GetTickTimeMS();
        auto & DI    = DeviceMap[PP.DeviceUuid];
        if (DI.DeviceUuid.empty()) {  // newly added device info
            assert(!DI.OnlineTimestampMS);
            X_DEBUG_PRINTF("new device");

            DI.DeviceUuid        = PP.DeviceUuid;
            DI.OnlineTimestampMS = NowMS;

            DI.Version            = PP.Version;
            DI.PrimaryIpv4Address = PP.PrimaryIpv4Address;
            DI.PrimaryIpv6Address = PP.PrimaryIpv6Address;

            DI.SupportUdpChannel  = PP.SupportUdpChannel;
            DI.SupportDnsRequests = PP.SupportDnsRequests;
            DI.SpeedLimitEnabled  = PP.SpeedLimitEnabled;

            // KeepAlive
            PostDeviceInfo(&DI, true);
            DI.LastKeepAliveTimestampMS = NowMS;
            DeviceTimeoutList.AddTail(DI);
        } else {
            X_DEBUG_PRINTF("device heartbeat");
            // TODO ("Accumulate audit info");

            // DI.TotalNewConnectionsSinceLastPost;
            // DI.TotalClosedConnectionSinceLastPost;
            // DI.TotalNewUdpChannelSinceLastPost;
            // DI.TotalClosedUdpChannelSinceLastPost;
            // DI.TotalDnsRequestSinceLastPost;

            // KeepAlive
            PostDeviceInfo(&DI, true);
            DI.LastKeepAliveTimestampMS = NowMS;
            DeviceTimeoutList.GrabTail(DI);
        }
    }

    bool OnServerPacket(xClientConnection & CC, xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) override {

        cout << "OnServerPacket: " << endl;
        cout << HexShow(PayloadPtr, PayloadSize) << endl;
        switch (CommandId) {
            case Cmd_DSR_DS_DeviceUpdate:
                cout << "Cmd_DSR_DS_DeviceUpdate" << endl;
                OnDeviceUpdate(PayloadPtr, PayloadSize);
                break;
        }

        return true;
    }

    void PostDeviceInfo(const xODI_DeviceInfo * DP, bool Online) {
        auto   Req   = xAD_BK_ReportDeviceInfoSingle();
        auto & ReqDI = Req.DeviceInfo;

        auto NowMS = GlobalTicker();

        Req.LocalAuditTimestampMS = NowMS;

        ReqDI.Version    = DP->Version;
        ReqDI.DeviceUuid = DP->DeviceUuid;
        // ReqDI.RelayServerUuid    = DP->RelayServerUuid;
        ReqDI.PrimaryIpv4Address = DP->PrimaryIpv4Address;
        ReqDI.PrimaryIpv6Address = DP->PrimaryIpv6Address;

        ReqDI.IsOffline          = !Online;
        ReqDI.SupportUdpChannel  = DP->SupportUdpChannel;
        ReqDI.SupportDnsRequests = DP->SupportDnsRequests;
        ReqDI.SpeedLimitEnabled  = DP->SpeedLimitEnabled;

        ReqDI.TotalOnlineTimeMS = NowMS - DP->OnlineTimestampMS;

        ubyte Buffer[MaxPacketSize];
        auto  MSize = WriteMessage(Buffer, Cmd_AuditTerminalInfo2, 0, Req);

        auto MsgKey = DP->DeviceUuid;
        KR.Post(MsgKey, Buffer, MSize);

        X_DEBUG_PRINTF("\n%s", HexShow(Buffer, MSize).c_str());
    }
};
static auto ODI = xODI();

int main(int argc, char ** argv) {

    auto ConfigLoader = GetConfigLoader(argc, argv);
    ConfigLoader.Require(Topic, "Topic");
    ConfigLoader.Require(SecurityProtocol, "SecurityProtocol");
    ConfigLoader.Require(SaslMechanism, "SaslMechanism");
    ConfigLoader.Require(SaslUsername, "SaslUsername");
    ConfigLoader.Require(SaslPassword, "SaslPassword");
    ConfigLoader.Require(BootstrapServers, "BootstrapServers");
    ConfigLoader.Require(DispatcherAddress, "DispatcherAddress");

    RuntimeAssert(KR.Init(
        Topic,
        {
            { "security.protocol", SecurityProtocol },
            { "sasl.mechanism", SaslMechanism },
            { "sasl.username", SaslUsername },
            { "sasl.password", SaslPassword },
            { "bootstrap.servers", BootstrapServers },
        }
    ));
    auto KRC = xScopeCleaner(KR);

    auto ODIG = xResourceGuard(ODI, &IC);
    RuntimeAssert(ODIG);
    ODI.AddServer(DispatcherAddress);

    while (true) {
        GlobalTicker.Update();
        IC.LoopOnce();
        ODI.Tick(GlobalTicker());
    }

    return 0;
}
