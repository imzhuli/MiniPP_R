#pragma once
#include "./_local_audit.hpp"
#include "./_mindef.hpp"
#include "./client_auth_cache_manager.hpp"
#include "./client_connection_manager.hpp"
#include "./device_selector_manager.hpp"
#include "./relay_connection_manager.hpp"
#include "./relay_server_list_manager.hpp"
#include "./test_relay_connection_manager.hpp"

#include <core/logger.hpp>

extern xNetAddress              ConfigExportBindAddress;
extern xNetAddress              ConfigTcpBindAddress;
extern xNetAddress              ConfigUdpBindAddress;
extern xNetAddress              ConfigAuthServerAddress;
extern xNetAddress              ConfigDeviceSelectorAddress;
extern std::vector<xNetAddress> ConfigCenterAddressList;
extern std::string              ConfigAuditLoggerFilename;

extern xRunState                   GlobalRunState;
extern xTicker                     GlobalTicker;
extern xPA_AuthCacheManager        GlobalAuthCacheManager;
extern xPA_RelayConnectionManager  GlobalRelayConnectionManager;
extern xPA_ClientConnectionManager GlobalClientConnectionManager;
extern xPA_RelayServerListManager  GlobalRelayServerListManager;
extern xPA_DeviceSelectorManager   GlobalDeviceSelectorManager;

extern xPA_LocalAudit GlobalLocalAudit;
extern xBaseLogger    GlobalAuditLogger;

extern std::map<uint64_t, xNetAddress> ConfigRelayServerMapForTest;
extern xPATest_RCM                     GlobalTestRCM;

extern void LoadConfig(const std::string & filename);
