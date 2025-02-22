#include "./_global.hpp"

xIoContext  GlobalIoContext;
xNetAddress BindAddressForDevice;
xNetAddress BindAddressForDeviceV6Test;

xUdpChannel Ipv6TestChennel;
xUdpChannel ChallengeChannel;

std::string IpLocationDbFilename;

xCC_IpLocationManager       IpLocationManager;
xCC_ChallengeContextManager ChallengeContextManager;
xCC_RelayServerManager      RelayServerManager;

std::vector<xCC_RelayServerBase> ForceRelayServerList;
