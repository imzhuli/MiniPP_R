#pragma once
#include "./device_manager.hpp"
#include "./device_reporter.hpp"
#include "./relay_connection_manager.hpp"
#include "./relay_service.hpp"

#include <pp_common/_.hpp>

extern std::string ServerUuid;
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
