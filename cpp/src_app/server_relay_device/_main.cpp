#include "./_config.hpp"
#include "./_global.hpp"
#include "./relay_service.hpp"

#include <pp_protocol/device_relay/init_ctrl_stream.hpp>

int main(int argc, char ** argv) {

    auto CL = xCommandLine(
        argc, argv,
        { {
            'c',
            nullptr,
            "config_file",
            true,
        } }
    );

    auto ConfigFileOpt = CL["config_file"];
    RuntimeAssert(ConfigFileOpt());
    RuntimeAssert(LoadConfig(ConfigFileOpt->c_str()));

    RuntimeAssert(GlobalIoContext.Init());
    RuntimeAssert(DeviceReporter.Init(&GlobalIoContext));
    RuntimeAssert(DeviceManager.Init(MaxDeviceCount));
    RuntimeAssert(DeviceConnectionManager.Init(&GlobalIoContext, MaxDeviceCount * 2));
    RuntimeAssert(DeviceRelayService.Init(&GlobalIoContext, BindCtrlAddress, BindDataAddress, BindProxyAddress));
    RuntimeAssert(ProxyConnectionManager.Init(&GlobalIoContext, MaxProxyCount));
    RuntimeAssert(RelayConnectionManager.Init(MaxRelayConnectionCount));

    DeviceReporter.AddServer(DeviceAuditAddress);

    auto LALG = xResourceGuard(LocalAuditLogger, LocalAuditFilename.c_str(), false);
    RuntimeAssert(LALG);

    auto AuditTimer = xTimer();
    while (true) {
        auto NowMS = GetTimestampMS();
        GlobalIoContext.LoopOnce();
        DeviceConnectionManager.Tick(NowMS);
        DeviceManager.Tick(NowMS);
        DeviceRelayService.Tick(NowMS);
        ProxyConnectionManager.Tick(NowMS);
        RelayConnectionManager.Tick(NowMS);
        DeviceReporter.Tick(NowMS);

        if (AuditTimer.TestAndTag(std::chrono::minutes(1))) {
            LocalAuditLogger.I("%s", LocalAudit.ToString().c_str());
            LocalAudit.ResetPeriodicalValues();
        }
    }

    RelayConnectionManager.Clean();
    ProxyConnectionManager.Clean();
    DeviceRelayService.Clean();
    DeviceConnectionManager.Clean();
    DeviceManager.Clean();
    DeviceReporter.Clean();
    GlobalIoContext.Clean();

    return 0;
}
