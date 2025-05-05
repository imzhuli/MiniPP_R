#include "./_global.hpp"

#include <config/config.hpp>

xNetAddress BindAddress;
xNetAddress DeviceDispatcherAddress;

xDR_DeviceContextManager  DeviceContextManager;
xDS_DeviceSelectorService DeviceSelectorService;
xDS_DeviceObserver        DeviceObserver;

void LoadConfig(const char * filename) {
    auto Loader = xConfigLoader(filename);
    RuntimeAssert(Loader);

    Loader.Require(BindAddress, "BindAddress");
    Loader.Require(DeviceDispatcherAddress, "DeviceDispatcherAddress");
}
