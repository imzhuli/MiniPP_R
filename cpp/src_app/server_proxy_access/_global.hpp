#pragma once
#include "./client_auth_cache_manager.hpp"
#include "./client_connection_manager.hpp"
#include "./relay_connection_manager.hpp"
#include "./relay_server_list_manager.hpp"

#include <pp_common/base.hpp>

extern xNetAddress              ConfigExportBindAddress;
extern xNetAddress              ConfigTcpBindAddress;
extern xNetAddress              ConfigUdpBindAddress;
extern std::vector<uint32_t>    ConfigLocalTagHashList;
extern std::vector<xNetAddress> ConfigCenterAddressList;

extern xRunState                   GlobalRunState;
extern xTicker                     GlobalTicker;
extern xPA_AuthCacheManager        GlobalAuthCacheManager;
extern xPA_RelayConnectionManager  GlobalRelayConnectionManager;
extern xPA_ClientConnectionManager GlobalClientConnectionManager;
extern xPA_RelayServerListManager  GlobalRelayServerListManager;

extern void LoadConfig(const std::string & filename);
