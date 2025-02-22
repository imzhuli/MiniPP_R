#pragma once

#include "./challenge.hpp"
#include "./ip_location.hpp"
#include "./relay_server_manager.hpp"

#include <pp_common/_.hpp>

extern xIoContext  GlobalIoContext;
extern xNetAddress BindAddressForDevice;
extern xNetAddress BindAddressForDeviceV6Test;

extern xUdpChannel Ipv6TestChennel;
extern xUdpChannel ChallengeChannel;

extern std::string IpLocationDbFilename;

extern xCC_IpLocationManager       IpLocationManager;
extern xCC_ChallengeContextManager ChallengeContextManager;
extern xCC_RelayServerManager      RelayServerManager;

extern std::vector<xCC_RelayServerBase> ForceRelayServerList;
