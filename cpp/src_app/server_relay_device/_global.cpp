#include "./_global.hpp"

xIoContext GlobalIoContext;

xNetAddress BindCtrlAddress;
xNetAddress BindDataAddress;
xNetAddress BindProxyAddress;

xNetAddress ExportCtrlAddress;
xNetAddress ExportDataAddress;
xNetAddress ExportProxyAddress;

xRD_DeviceConnectionManager DeviceConnectionManager;
xDeviceManager              DeviceManager;
xRD_ProxyConnectionManager  ProxyConnectionManager;
xDeviceRelayService         DeviceRelayService;
xRelayConnectionManager     RelayConnectionManager;
