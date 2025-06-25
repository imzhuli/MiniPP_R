#include "../lib_server_util/extra/auth_cache_server_list_client.hpp"
#include "./_global.hpp"

auto IC  = xIoContext();
auto ICG = xResourceGuard(IC);

xDownloadAuthCacheServerListClient Client;

int main(int argc, char ** argv) {
    auto REG = xRuntimeEnvGuard(argc, argv);
    LoadConfig();

    auto CG = xResourceGuard(Client, &IC, ServerListServerAddress);
    RuntimeAssert(CG);

    while (true) {
        ServiceTicker.Update();
        IC.LoopOnce();
        TickAll(ServiceTicker(), Client);
    }

    return 0;
    //
}
