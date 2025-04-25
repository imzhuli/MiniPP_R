#pragma once

#include "./backend_server_manager.hpp"
#include "./challenge.hpp"
#include "./ip_location.hpp"
#include "./pa_config.hpp"
#include "./relay_server_manager.hpp"

#include <pp_common/_.hpp>

extern xIoContext  GlobalIoContext;
extern xNetAddress BindAddressForDevice;
extern xNetAddress BindAddressForDeviceV6Test;
extern xNetAddress BindAddressForProxyAccess;
extern xNetAddress BindAddressForGetBackendServerList;

extern xUdpChannel Ipv6TestChennel;
extern xUdpChannel ChallengeChannel;

extern std::string IpLocationDbFilename;
extern std::string BackendServerListFilename;

extern xCC_IpLocationManager       IpLocationManager;
extern xCC_BackendServerManager    BackendServerManager;
extern xCC_ChallengeContextManager ChallengeContextManager;
extern xCC_RelayServerManager      RelayServerManager;
extern xCC_PAConfigManager         PAConfigManager;

extern std::vector<xCC_RelayServerBase> ForceRelayServerList;
