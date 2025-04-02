#include "./_global.hpp"

xNetAddress           ConfigExportBindAddress;
xNetAddress           ConfigTcpBindAddress;
xNetAddress           ConfigExportUdpBindAddress;
xNetAddress           ConfigUdpBindAddress;
xNetAddress           ConfigConfigCenterAddress;
std::vector<uint32_t> ConfigLocalTagHashList;

xRunState                   GlobalRunState;
xTicker                     GlobalTicker;
xPA_RelayConnectionManager  GlobalRelayConnectionManager;
xPA_ClientConnectionManager GlobalClientConnectionManager;

void LoadConfig(const std::string & filename) {
}
