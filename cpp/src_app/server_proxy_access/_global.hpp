#pragma once
#include "./client_connection_manager.hpp"
#include "./relay_connection_manager.hpp"

#include <pp_common/base.hpp>

extern xNetAddress           ConfigExportBindAddress;
extern xNetAddress           ConfigTcpBindAddress;
extern xNetAddress           ConfigUdpBindAddress;
extern std::vector<uint32_t> ConfigLocalTagHashList;

extern xRunState                   GlobalRunState;
extern xTicker                     GlobalTicker;
extern xPA_RelayConnectionManager  GlobalRelayConnectionManager;
extern xPA_ClientConnectionManager GlobalClientConnectionManager;

extern void LoadConfig(const std::string & filename);
