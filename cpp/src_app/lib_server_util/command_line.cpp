#include "./command_line.hpp"

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
