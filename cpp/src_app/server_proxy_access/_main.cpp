#include "./_global.hpp"

#include <iostream>
#include <pp_common/base.hpp>

auto IC = xIoContext();

int main(int argc, char ** argv) {

    auto ICG  = xResourceGuard(IC);
    auto RCMG = xResourceGuard(GlobalRelayConnectionManager, &IC);

    GlobalRunState.Start();
    GlobalRelayConnectionManager.AddRelayGroup(1234, xNetAddress::Parse("127.0.0.1:10000"));

    while (GlobalRunState) {
        GlobalTicker.Update();
        IC.LoopOnce();
        GlobalRelayConnectionManager.Tick(GlobalTicker());
    }
    GlobalRunState.Finish();

    return 0;
}
