#include "./_global.hpp"

#include <iostream>
#include <pp_common/base.hpp>

auto IC = xIoContext();

int main(int argc, char ** argv) {

    auto CL = xCommandLine(
        argc, argv,
        {
            { 'c', "config", "config", true },
        }
    );
    auto ConfigFilename = CL["config"];
    RuntimeAssert(ConfigFilename());
    LoadConfig(ConfigFilename->c_str());

    auto ICG   = xResourceGuard(IC);
    auto CAMG  = xResourceGuard(GlobalAuthCacheManager, &IC, ConfigCenterAddressList[0]);
    auto RCMG  = xResourceGuard(GlobalRelayConnectionManager, &IC);
    auto RSLMG = xResourceGuard(GlobalRelayServerListManager, &IC, ConfigCenterAddressList);

    GlobalRunState.Start();

    while (GlobalRunState) {
        GlobalTicker.Update();
        IC.LoopOnce();
        TickAll(GlobalTicker(), GlobalAuthCacheManager, GlobalRelayConnectionManager, GlobalRelayServerListManager);
    }
    GlobalRunState.Finish();

    return 0;
}
