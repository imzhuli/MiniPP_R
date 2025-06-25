#pragma once
#include "../lib_server_util/base.hpp"
#include "./register_server_client.hpp"

#include <pp_common/base.hpp>

extern xNetAddress BindAddress;
extern xNetAddress ExportServerAddress;

extern xNetAddress ServerIdCenterAddress;
extern xNetAddress ServerListRegisterAddress;

extern xAD_xServerIdClient      ServerIdClient;
extern xAD_RegisterServerClient RegisterServerClient;

extern void LoadConfig();
