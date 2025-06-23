#include "./_global.hpp"

xNetAddress BindAddress;
xNetAddress ExportServerAddress;

xNetAddress ServerIdCenterAddress;
xNetAddress ServerListRegisterAddress;

uint64_t CacheNodeTimeoutMS = 10 * 60'000;

xAC_xServerIdClient      ServerIdClient;
xAC_RegisterServerClient RegisterServerClient;