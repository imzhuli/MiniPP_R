#include "./global.hpp"

#include "./auth_backends.hpp"

#include <config/config.hpp>

xNetAddress BindAddress;
std::string AuthBackendServerListFile;

void LoadConfig(const char * filename) {
    auto CL = xConfigLoader(filename);
    CL.Require(BindAddress, "BindAddress");
    CL.Require(AuthBackendServerListFile, "AuthBackendServerListFile");

    LoadAuthBackendServerList(AuthBackendServerListFile.c_str());
}
