#include "./_global.hpp"

xIoContext  GlobalIoContext;
xNetAddress BindAddressForDevice;
xNetAddress BindAddressForDeviceV6Test;
xNetAddress BindAddressForProxyAccess;
xNetAddress BindAddressForGetBackendServerList;

xUdpChannel Ipv6TestChennel;
xUdpChannel ChallengeChannel;

std::string GeoInfoMapFilename;
std::string IpLocationDbFilename;
std::string BackendServerListFilename;

xCC_IpLocationManager       IpLocationManager;
xCC_BackendServerManager    BackendServerManager;
xCC_ChallengeContextManager ChallengeContextManager;
xCC_RelayServerManager      RelayServerManager;
xCC_PAConfigManager         PAConfigManager;

std::vector<xCC_RelayServerBase> ForceRelayServerList;
