#pragma once
#include "../lib_server_util/base.hpp"
#include "./register_server_client.hpp"

#include <pp_common/base.hpp>

extern xNetAddress BindAddress;
extern xNetAddress ExportServerAddress;

extern xNetAddress ServerIdCenterAddress;
extern xNetAddress ServerListRegisterAddress;

extern uint64_t CacheNodeTimeoutMS;
extern uint64_t LocalServerId;

extern xAC_xServerIdClient      ServerIdClient;
extern xAC_RegisterServerClient RegisterServerClient;
