#include "./_global.hpp"

#include <pp_common/_.hpp>

static auto IC  = xIoContext();
static auto ICG = xResourceGuard(IC);

int main(int argc, char ** argv) {

    auto SG = xRuntimeEnvGuard(argc, argv);
    LoadConfig();

    auto SICG = xResourceGuard(ServerIdClient, &IC, ServerIdCenterAddress);
    auto RSCG = xResourceGuard(RegisterServerClient, &IC, ServerListRegisterAddress);

    while (true) {
        ServiceTicker.Update();
        IC.LoopOnce();

        TickAll(ServiceTicker(), ServerIdClient, RegisterServerClient);
    }

    return 0;
}
