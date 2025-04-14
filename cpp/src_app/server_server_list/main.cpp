#include "./auth_cache_server_manager.hpp"
#include "./global.hpp"

auto IC  = xIoContext();
auto ICG = xResourceGuard(IC);

int main(int argc, char ** argv) {

    auto CL = xCommandLine(
        argc, argv,
        {
            { 'c', "config", "config", true },
        }
    );

    auto OptC = CL["config"];
    RuntimeAssert(OptC(), "require config filename");
    LoadConfig(OptC->c_str());

    RuntimeAssert(ICG);

    auto MS = xCC_AuthCacheServerManager();
    Touch(MS);

    return 0;
}
