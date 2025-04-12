#include <pp_common/_.hpp>
//
#include <core/timer_wheel.hpp>
#include <iostream>
#include <network/net_address.hpp>
#include <pp_protocol/dns_server/dns_dispatcher.hpp>

using namespace std;
using namespace xel;

int main(int, char **) {
    xIndexedStorage<int> IS;
    IS.Init(1024);

    auto Id = IS.Acquire(0);
    IS.Release(Id);
    RuntimeAssert(!IS.Check(Id));

    int i = 0;
    int j = 0;

    Reset(i, 1024);
    RuntimeAssert(i == 1024);

    j = Steal(i, 777);
    RuntimeAssert(j == 1024);
    RuntimeAssert(i == 777);

    j = Steal(i);
    RuntimeAssert(j == 777);
    RuntimeAssert(i == 0);

    Reset(i, 666);
    RuntimeAssert(i == 666);
    Reset(i);
    RuntimeAssert(i == 0);

    return 0;
}
