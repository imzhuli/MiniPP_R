#include "../lib_server_util/all.hpp"

#include <pp_common/_.hpp>

static auto IC     = xIoContext();
static auto ICG    = xResourceGuard(IC);
static auto Ticker = xTicker();

int main(int, char **) {

    auto P = xServiceClientHashPool();
    P.Init(&IC);

    auto a1 = xServiceClientHashPool::xServerInfo{ 1, xNetAddress::Parse("127.0.0.1:10000") };
    auto a2 = xServiceClientHashPool::xServerInfo{ 2, xNetAddress::Parse("127.0.0.1:10000") };
    auto a3 = xServiceClientHashPool::xServerInfo{ 3, xNetAddress::Parse("127.0.0.1:10000") };
    auto a4 = xServiceClientHashPool::xServerInfo{ 4, xNetAddress::Parse("127.0.0.1:10000") };
    auto a5 = xServiceClientHashPool::xServerInfo{ 5, xNetAddress::Parse("127.0.0.1:10000") };
    auto a6 = xServiceClientHashPool::xServerInfo{ 6, xNetAddress::Parse("127.0.0.1:10000") };

    P.UpdateServerList({ a1, a2, a3 });
    for (auto & R : P.ActiveServerList) {
        X_DEBUG_PRINTF("P->Id:%i, @%s", (int)R->ServerId, R->Address.ToString().c_str());
    }
    cout << "________________" << endl;
    P.Tick(1);
    for (auto & R : P.ActiveServerList) {
        X_DEBUG_PRINTF("P->Id:%i, @%s", (int)R->ServerId, R->Address.ToString().c_str());
    }

    cout << "________________" << endl;
    P.UpdateServerList({ a2, a4, a5, a6 });
    for (auto & R : P.ActiveServerList) {
        X_DEBUG_PRINTF("P->Id:%i, @%s", (int)R->ServerId, R->Address.ToString().c_str());
    }
    cout << "________________" << endl;
    P.Tick(1);
    for (auto & R : P.ActiveServerList) {
        X_DEBUG_PRINTF("P->Id:%i, @%s", (int)R->ServerId, R->Address.ToString().c_str());
    }

    auto T = xTimer();
    while (!T.TestAndTag(100s)) {
        Ticker.Update();
        IC.LoopOnce();
        TickAll(Ticker(), P);
    }

    P.Clean();

    return 0;
}
