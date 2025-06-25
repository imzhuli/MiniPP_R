#include "./_global.hpp"

xNetAddress ServerListServerAddress;

void LoadConfig() {

    auto CL = xConfigLoader(RuntimeEnv.DefaultConfigFilePath);
    CL.Require(ServerListServerAddress, "ServerListServerAddress");

    //
}
