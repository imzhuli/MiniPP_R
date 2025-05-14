#pragma once
#include "./device_manager.hpp"
#include "./device_reporter.hpp"
#include "./relay_connection_manager.hpp"
#include "./relay_service.hpp"

#include <pp_common/_.hpp>

extern std::string ServerUuid;
extern uint64_t    ServerRuntimeId;
extern xIoContext  GlobalIoContext;

extern xNetAddress BindCtrlAddress;
extern xNetAddress BindDataAddress;
extern xNetAddress BindProxyAddress;

extern xNetAddress ExportCtrlAddress;
extern xNetAddress ExportDataAddress;
extern xNetAddress ExportProxyAddress;

extern xNetAddress        DeviceAuditAddress;
extern xRD_DeviceReporter DeviceReporter;

extern xRD_DeviceConnectionManager DeviceConnectionManager;
extern xDeviceManager              DeviceManager;
extern xRD_ProxyConnectionManager  ProxyConnectionManager;
extern xDeviceRelayService         DeviceRelayService;
extern xRD_RelayConnectionManager  RelayConnectionManager;

extern struct xRD_LocalAudit {
    uint64_t TotalCtrlConnections;
    uint64_t TotalDataConnections;
    uint64_t TotalDeviceEnabled;
    uint64_t TotalRelayConnections;

    uint64_t NewRelayConnections;
    uint64_t DeviceClosedConnections;
    uint64_t ProxyClosedConnections;

    void        ResetPeriodicalValues();
    std::string ToString() const;
} LocalAudit;
extern std::string LocalAuditFilename;
extern xBaseLogger LocalAuditLogger;
