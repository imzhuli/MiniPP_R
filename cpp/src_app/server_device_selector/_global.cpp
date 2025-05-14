#include "./_global.hpp"

#include <config/config.hpp>

xNetAddress BindAddress;
xNetAddress DeviceDispatcherAddress;

xDS_DeviceContextManager  DeviceContextManager;
xDS_DeviceSelectorService DeviceSelectorService;
xDS_DeviceObserver        DeviceObserver;

xDS_LocalAudit LocalAudit;
std::string    AuditLoggerFilename;
xBaseLogger    AuditLogger;

void xDS_LocalAudit::ResetPeriodCount() {
    Reset(NewDeviceCount);
    Reset(ReplacedDeviceCount);
    Reset(RemovedDeviceCount);
    Reset(TimeoutDeviceCount);
}

#define O(x) OS << #x << "=" << (x) << ' '
std::string xDS_LocalAudit::ToString() const {
    auto OS = std::ostringstream();

    O(NewDeviceCount);
    O(ReplacedDeviceCount);
    O(RemovedDeviceCount);
    O(TimeoutDeviceCount);

    O(TotalDeviceCount);

    return OS.str();
}

void LoadConfig(const char * filename) {
    auto Loader = xConfigLoader(filename);
    RuntimeAssert(Loader);

    Loader.Require(BindAddress, "BindAddress");
    Loader.Require(DeviceDispatcherAddress, "DeviceDispatcherAddress");
    Loader.Require(AuditLoggerFilename, "AuditLoggerFilename");
}
