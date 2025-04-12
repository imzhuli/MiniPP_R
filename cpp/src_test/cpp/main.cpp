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
    cout << YN(i == 1024) << endl;

    j = Steal(i, 777);
    cout << YN(j == 1024) << endl;
    cout << YN(i == 777) << endl;

    j = Steal(i);
    cout << YN(j == 777) << endl;
    cout << YN(i == 0) << endl;

    Reset(i, 666);
    cout << YN(i == 666) << endl;
    Reset(i);
    cout << YN(i == 0) << endl;

    return 0;
}
