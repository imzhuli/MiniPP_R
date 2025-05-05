#pragma once
#include <config/config.hpp>
#include <pp_common/base.hpp>

std::string   GetConfigFile(int CmdArgc, char ** CmdArgv);
xConfigLoader GetConfigLoader(int CmdArgc, char ** CmdArgv);