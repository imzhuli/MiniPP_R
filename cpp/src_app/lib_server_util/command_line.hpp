#pragma once
#include <config/config.hpp>
#include <filesystem>
#include <pp_common/base.hpp>

struct xRuntimeEnv {
    std::string           ProgramName;
    std::filesystem::path HomeDir;
    std::filesystem::path BinDir;
    std::filesystem::path ConfigDir;
    std::filesystem::path DataDir;
    std::filesystem::path CacheDir;  //

    std::string ToString() const;
};

xRuntimeEnv ParseEnv(int CmdArgc, char ** CmdArgv);

std::string   GetConfigFile(int CmdArgc, char ** CmdArgv);
xConfigLoader GetConfigLoader(int CmdArgc, char ** CmdArgv);