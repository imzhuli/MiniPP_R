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
    auto CAMG  = xResourceGuard(GlobalAuthCacheManager, &IC, ConfigAuthServerAddress);
    auto RCMG  = xResourceGuard(GlobalRelayConnectionManager, &IC);
    auto RSLMG = xResourceGuard(GlobalRelayServerListManager, &IC, ConfigCenterAddressList);
    auto CCMG  = xResourceGuard(GlobalClientConnectionManager, &IC, ConfigTcpBindAddress, 10'0000);
    auto DSMG  = xResourceGuard(GlobalDeviceSelectorManager, &IC);

    GlobalDeviceSelectorManager.AddServer(ConfigDeviceSelectorAddress);

    GlobalRunState.Start();

    auto AuditTimer = xTimer();
    while (GlobalRunState) {
        GlobalTicker.Update();
        IC.LoopOnce();
        TickAll(GlobalTicker(), GlobalAuthCacheManager, GlobalRelayConnectionManager, GlobalRelayServerListManager, GlobalClientConnectionManager, GlobalDeviceSelectorManager);

        if (AuditTimer.TestAndTag(std::chrono::seconds(30))) {
            cout << "PA_LocalAudit: " << endl;
            cout << GlobalLocalAudit.ToString() << endl;
        }
    }
    GlobalRunState.Finish();

    return 0;
}
