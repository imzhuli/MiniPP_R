#include "../lib_server_util/all.hpp"
#include "./_global.hpp"

#include <functional>
#include <pp_common/_.hpp>
#include <pp_protocol/command.hpp>
#include <pp_protocol/internal/device_state.hpp>
#include <unordered_set>

static auto TargetAddress = xNetAddress();
static auto IC            = xIoContext();
static auto ICG           = xResourceGuard(IC);

static auto KR               = xKfkProducer();
static auto SecurityProtocol = std::string();
static auto SaslMechanism    = std::string();
static auto SaslUsername     = std::string();
static auto SaslPassword     = std::string();
static auto BootstrapServers = std::string();

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
            //
            Todo("Post offline device info");

            DeviceMap.erase(Iter);
            return;
        }

        auto   NowMS = GetTickTimeMS();
        auto & DI    = DeviceMap[PP.DeviceUuid];
        if (!DI.DeviceUuid.empty()) {  // newly added device info
            assert(!DI.OnlineTimestampMS);

            DI.OnlineTimestampMS = NowMS;
            Todo("Init device node");

            DI.LastKeepAliveTimestampMS = NowMS;
            DeviceTimeoutList.AddTail(DI);

            // KeepAlive
            Todo("Post device online info");
        } else {
            Todo("Accumulate audit info");

            // KeepAlive
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

    //
};
static auto ODI = xODI();

int main(int argc, char ** argv) {

    auto ConfigLoader = GetConfigLoader(argc, argv);
    ConfigLoader.Require(SecurityProtocol, "SecurityProtocol");
    ConfigLoader.Require(SaslMechanism, "SaslMechanism");
    ConfigLoader.Require(SaslUsername, "SaslUsername");
    ConfigLoader.Require(SaslPassword, "SaslPassword");
    ConfigLoader.Require(BootstrapServers, "BootstrapServers");

    RuntimeAssert(KR.Init(
        "my-topic",
        {
            { "security.protocol", SecurityProtocol },
            { "sasl.mechanism", SaslMechanism },
            { "sasl.username", SaslUsername },
            { "sasl.password", SaslPassword },
            { "bootstrap.servers", BootstrapServers },
        }
    ));
    auto KRC = xScopeCleaner(KR);

    // RuntimeAssert(InitKafka());
    // auto KFKG = xScopeGuard(CleanKafka);

    for (size_t I = 0; I < 100; ++I) {
        auto Payload = std::string("Message: ") + std::to_string(I);
        KR.Post(Payload);
    }
    KR.Flush();

    std::this_thread::sleep_for(std::chrono::seconds(5));

    // auto CL = GetConfigLoader(argc, argv);
    // CL.Require(TargetAddress, "TargetAddress");

    // auto ODIG = xResourceGuard(ODI, &IC);
    // RuntimeAssert(ODIG);

    // while (true) {
    //     IC.LoopOnce();
    // }

    return 0;
}
