#include "../lib_server_util/all.hpp"

#include <pp_common/_.hpp>
#include <pp_protocol/command.hpp>

struct xODI : public xClientPool {
    using xClientPool::Clean;
    using xClientPool::Init;

    bool OnServerPacket(xClientConnection & CC, xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) override {

        cout << "OnServerPacket: " << endl;
        cout << HexShow(PayloadPtr, PayloadSize) << endl;
        switch (CommandId) {
            case Cmd_DSR_DS_DeviceUpdate:
                cout << "Cmd_DSR_DS_DeviceUpdate" << endl;
                break;
        }

        return true;
    }

    //
};

static auto TargetAddress = xNetAddress();
static auto IC            = xIoContext();
static auto ICG           = xResourceGuard(IC);
static auto ODI           = xODI();

int main(int argc, char ** argv) {
    auto CL = GetConfigLoader(argc, argv);
    CL.Require(TargetAddress, "TargetAddress");

    auto ODIG = xResourceGuard(ODI, &IC);
    RuntimeAssert(ODIG);

    while (true) {
        IC.LoopOnce();
    }

    return 0;
}
