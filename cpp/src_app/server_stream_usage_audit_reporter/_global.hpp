#pragma once
#include "../lib_server_util/all.hpp"

extern xNetAddress ServerIdCenterAddress;
extern uint64_t    LocalServerId;

extern std::vector<xNetAddress> DispatcherAddressList;

extern std::string SecurityProtocol;
extern std::string SaslMechanism;
extern std::string SaslUsername;
extern std::string SaslPassword;
extern std::string BootstrapServerList;
extern std::string Topic;

extern std::string LoggerFilename;

extern void InitLogger();
extern void CleanLogger();
extern void LoadConfig();
