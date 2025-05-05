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
    RuntimeAssert(DSSG);

    auto DSDOG = xResourceGuard(DeviceObserver, &IC);
    RuntimeAssert(DSDOG);
    RuntimeAssert(DeviceObserver.AddServer(DeviceDispatcherAddress));

    while (true) {
        IC.LoopOnce();
        DeviceSelectorService.Tick(Ticker());
        DeviceObserver.Tick(Ticker());
        DeviceContextManager.Tick(Ticker());
    }
    return 0;
}
