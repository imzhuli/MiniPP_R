#pragma once
#include <config/config.hpp>
#include <pp_common/base.hpp>

// c++ headers
#include <filesystem>

struct xRuntimeEnv {
    std::filesystem::path HomeDir;
    std::filesystem::path BinDir;
    std::filesystem::path ConfigDir;
    std::filesystem::path DataDir;
    std::filesystem::path CacheDir;  //

    std::string           ProgramName;
    std::filesystem::path DefaultConfigFilePath;

    std::filesystem::path GetBinaryPath(const std::filesystem::path & Filename) const;
    std::filesystem::path GetConfigPath(const std::filesystem::path & Filename) const;
    std::filesystem::path GetDataPath(const std::filesystem::path & Filename) const;
    std::filesystem::path GetCachePath(const std::filesystem::path & Filename) const;

    static xRuntimeEnv FromCommandLine(int CmdArgc, char ** CmdArgv);
};
std::string ToString(const xRuntimeEnv & Env);

std::string   GetConfigFile(int CmdArgc, char ** CmdArgv);
xConfigLoader GetConfigLoader(int CmdArgc, char ** CmdArgv);