#pragma once
#include "./client_connection_manager.hpp"
#include "./relay_connection_manager.hpp"

#include <pp_common/base.hpp>

extern xNetAddress ConfigTcpBindAddress;
extern xNetAddress ConfigUdpBindAddress;

extern xRunState                   GlobalRunState;
extern xTicker                     GlobalTicker;
extern xPA_RelayConnectionManager  GlobalRelayConnectionManager;
extern xPA_ClientConnectionManager GlobalClientConnectionManager;
