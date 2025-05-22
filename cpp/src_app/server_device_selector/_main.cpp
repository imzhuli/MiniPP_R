#include "../lib_server_util/all.hpp"
#include "./_global.hpp"
#include "./device_context_manager.hpp"

#include <pp_protocol/command.hpp>

static auto IC     = xIoContext();
static auto ICG    = xResourceGuard(IC);
static auto Ticker = xTicker();

int main(int argc, char ** argv) {
    auto C = GetConfigFile(argc, argv);
    LoadConfig(C.c_str());

    auto DSSG = xResourceGuard(DeviceSelectorService, &IC, BindAddress, MAX_RELAY_DEVICE_SERVER_SUPPORTED, true);
    auto LAG  = xResourceGuard(AuditLogger, AuditLoggerFilename.c_str(), false);
    RuntimeAssert(DSSG);
    RuntimeAssert(LAG);

    auto DSDOG = xResourceGuard(DeviceObserver, &IC);
    RuntimeAssert(DSDOG);
    RuntimeAssert(DeviceObserver.AddServer(DeviceDispatcherAddress));

    auto AuditTimestampMS = Ticker();
    while (true) {
        Ticker.Update();
        IC.LoopOnce();
        DeviceSelectorService.Tick(Ticker());
        DeviceObserver.Tick(Ticker());
        DeviceContextManager.Tick(Ticker());

        LocalAudit.DurationMS = Ticker() - AuditTimestampMS;
        if (LocalAudit.DurationMS >= 60'000) {
            AuditLogger.I("%s", LocalAudit.ToString().c_str());
            LocalAudit.ResetPeriodCount();
            AuditTimestampMS = Ticker();
        }
    }
    return 0;
}
