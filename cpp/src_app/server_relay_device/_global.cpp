#include "./_global.hpp"

std::string ServerUuid      = {};
uint64_t    ServerRuntimeId = {};
xIoContext  GlobalIoContext = {};

xNetAddress BindCtrlAddress;
xNetAddress BindDataAddress;
xNetAddress BindProxyAddress;

xNetAddress ExportCtrlAddress;
xNetAddress ExportDataAddress;
xNetAddress ExportProxyAddress;

xNetAddress        DeviceAuditAddress;
xRD_DeviceReporter DeviceReporter;

xRD_DeviceConnectionManager DeviceConnectionManager;
xDeviceManager              DeviceManager;
xRD_ProxyConnectionManager  ProxyConnectionManager;
xDeviceRelayService         DeviceRelayService;
xRD_RelayConnectionManager  RelayConnectionManager;
