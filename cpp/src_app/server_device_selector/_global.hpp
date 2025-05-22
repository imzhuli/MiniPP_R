#pragma once
#include "./device_context_manager.hpp"
#include "./device_selector.hpp"

#include <pp_common/base.hpp>

extern xNetAddress BindAddress;
extern xNetAddress DeviceDispatcherAddress;

extern xDS_DeviceContextManager  DeviceContextManager;
extern xDS_DeviceSelectorService DeviceSelectorService;
extern xDS_DeviceObserver        DeviceObserver;

extern struct xDS_LocalAudit {
    uint64_t DurationMS = 0;

    size_t NewDeviceCount      = 0;
    size_t ReplacedDeviceCount = 0;
    size_t RemovedDeviceCount  = 0;
    size_t TimeoutDeviceCount  = 0;

    size_t TotalDeviceCount = 0;

    void        ResetPeriodCount();
    std::string ToString() const;
} LocalAudit;

extern std::string AuditLoggerFilename;
extern xBaseLogger AuditLogger;

extern void LoadConfig(const char * filename);
