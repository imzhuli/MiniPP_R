#include "./_global.hpp"

#include <config/config.hpp>
#include <fstream>

xNetAddress ServerIdCenterAddress;
uint64_t    LocalServerId = 0;

std::vector<xNetAddress> DispatcherAddressList;

std::string SecurityProtocol;
std::string SaslMechanism;
std::string SaslUsername;
std::string SaslPassword;
std::string BootstrapServerList;
std::string Topic;

std::string LoggerFilename;

void LoadConfig() {
    auto CL = xConfigLoader(RuntimeEnv.DefaultConfigFilePath.c_str());
    CL.Require(ServerIdCenterAddress, "ServerIdCenterAddress");

    auto DispatcherAddressListStr = std::string();
    CL.Require(DispatcherAddressListStr, "DispatcherAddressList");

    auto DispatcherAddressListOpt = ParsePythonStringArray(DispatcherAddressListStr);
    RuntimeAssert(DispatcherAddressListOpt(), "Require DispatcherAddressList (python string array format)");
    Reset(DispatcherAddressList);
    for (auto & S : *DispatcherAddressListOpt) {
        auto Addr = xNetAddress::Parse(S);
        RuntimeAssert(Addr && Addr.Port);
        DispatcherAddressList.push_back(Addr);
    }
    std::sort(DispatcherAddressList.begin(), DispatcherAddressList.end());

    CL.Require(SecurityProtocol, "SecurityProtocol");
    CL.Require(SaslMechanism, "SaslMechanism");
    CL.Require(SaslUsername, "SaslUsername");
    CL.Require(SaslPassword, "SaslPassword");
    CL.Require(BootstrapServerList, "BootstrapServerList");
    CL.Require(Topic, "Topic");

    CL.Optional(LoggerFilename, "LoggerFilename", "stream_usage_audit_reporter.log");
}
