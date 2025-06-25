#pragma once
#include "../lib_server_util/base.hpp"
#include "./register_server_client.hpp"

#include <pp_common/base.hpp>

extern xNetAddress BindAddress;
extern xNetAddress ExportServerAddress;

extern xNetAddress ServerIdCenterAddress;
extern xNetAddress ServerListRegisterAddress;

extern xAA_xServerIdClient      ServerIdClient;
extern xAA_RegisterServerClient RegisterServerClient;

extern void LoadConfig();
