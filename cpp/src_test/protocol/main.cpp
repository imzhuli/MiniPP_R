#include <iostream>
#include <pp_protocol/ac_cc/alloc_cache_server_id.hpp>
using namespace xel;
using namespace std;

int main(int argc, char ** argv) {

    auto S   = x_AC_CC_AllocServerId();
    auto Now = GetTimestampMS();

    S.TimestampMS = Now;
    ubyte B[1024];
    auto  RS = S.Serialize(B, sizeof(B));
    cout << "RS=" << RS << endl;
    cout << HexShow(B, RS) << endl;

    auto DS = S.Deserialize(B, RS);
    cout << "DS=" << DS << endl;

    return 0;
}
