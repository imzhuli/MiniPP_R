#include "./_global.hpp"

xNetAddress BindAddress;
xNetAddress ExportServerAddress;

xNetAddress ServerIdCenterAddress;
xNetAddress ServerListRegisterAddress;

xAD_xServerIdClient      ServerIdClient;
xAD_RegisterServerClient RegisterServerClient;

void LoadConfig() {
    auto CL = xConfigLoader(RuntimeEnv.DefaultConfigFilePath);

    CL.Require(BindAddress, "BindAddress");
    CL.Require(ExportServerAddress, "ExportServerAddress");
    CL.Require(ServerIdCenterAddress, "ServerIdCenterAddress");
    CL.Require(ServerListRegisterAddress, "ServerListRegisterAddress");
}
