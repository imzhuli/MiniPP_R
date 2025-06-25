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

xNetAddress        AuditDeviceAddress;
xRD_DeviceReporter DeviceReporter;

xRD_DeviceConnectionManager DeviceConnectionManager;
xDeviceManager              DeviceManager;
xRD_ProxyConnectionManager  ProxyConnectionManager;
xDeviceRelayService         DeviceRelayService;
xRD_RelayConnectionManager  RelayConnectionManager;

xRD_LocalAudit LocalAudit;

void xRD_LocalAudit::ResetPeriodicalValues() {

    Reset(NewRelayConnections);
    Reset(DeviceClosedConnections);
    Reset(ProxyClosedConnections);

    //
}

#define O(x) OS << #x << "=" << (x) << ' '
std::string xRD_LocalAudit::ToString() const {

    auto OS = std::ostringstream();

    O(TotalCtrlConnections);
    O(TotalDataConnections);
    O(TotalDeviceEnabled);
    O(TotalRelayConnections);

    O(NewRelayConnections);
    O(DeviceClosedConnections);
    O(ProxyClosedConnections);

    return OS.str();
}
