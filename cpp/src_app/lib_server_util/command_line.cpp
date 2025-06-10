#include "./command_line.hpp"

#include <filesystem>
#include <sstream>

std::string xRuntimeEnv::ToString() const {
    auto OS = std::ostringstream();
    OS << "Home: " << HomeDir << endl;
    OS << "Bin: " << BinDir << endl;
    OS << "Conf: " << ConfigDir << endl;
    OS << "Data: " << DataDir << endl;
    OS << "Cache: " << CacheDir << endl;
    return OS.str();
}

xRuntimeEnv ParseEnv(int CmdArgc, char ** CmdArgv) {
    auto Env = xRuntimeEnv{};
    auto CL  = xCommandLine(
        CmdArgc, CmdArgv,
        {
            { 'c', "config", "config-file", true },
            { 'h', "home", "home_dir", true },
        }
    );
    if (!CmdArgc) {
        return Env;
    }
    Env.ProgramName = CmdArgv[0];

    auto DefaultBinPath = std::filesystem::current_path();
    auto HomeOpt        = CL["home_dir"];
    if (!HomeOpt()) {
        Env.HomeDir   = std::filesystem::absolute(DefaultBinPath);
        Env.BinDir    = Env.HomeDir;
        Env.ConfigDir = Env.HomeDir;
        Env.DataDir   = Env.HomeDir;
        Env.CacheDir  = Env.HomeDir;
    } else {
        Env.HomeDir   = std::filesystem::absolute(*HomeOpt);
        Env.BinDir    = Env.HomeDir / "bin";
        Env.ConfigDir = Env.HomeDir / "conf";
        Env.DataDir   = Env.HomeDir / "data";
        Env.CacheDir  = Env.HomeDir / "cache";
    }
    return Env;
}

std::string GetConfigFile(int CmdArgC, char ** CmdArgV) {
    auto CL = xCommandLine(
        CmdArgC, CmdArgV,
        {
            { 'c', "config", "config-file", true },
        }
    );
    auto Opt = CL["config-file"];
    if (!Opt()) {
        return {};
    }
    return *Opt;
}

xConfigLoader GetConfigLoader(int argc, char ** argv) {
    auto Filename = GetConfigFile(argc, argv);
    return xConfigLoader(Filename.c_str());
}
