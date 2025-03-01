#include "./_global.hpp"

#include <config/config.hpp>

xNetAddress              BindAddress;
xNetAddress              ReportDispatcherAddress;
std::vector<xNetAddress> RequestDispatcherAddress;

void LoadConfig(const char * filename) {
    auto Loader = xConfigLoader(filename);
    RuntimeAssert(Loader);

    Loader.Require(BindAddress, "BindAddress");
}
