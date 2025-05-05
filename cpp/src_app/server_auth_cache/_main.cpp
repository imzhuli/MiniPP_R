#include "../lib_backend_connection/backend_connection_pool.hpp"

#include <pp_protocol/_backend/auth_by_user_pass.hpp>
#include <pp_protocol/command.hpp>

struct xAuthTest : public xBackendConnectionPool {

    using xBackendConnectionPool::Clean;
    using xBackendConnectionPool::Init;

    bool OnBackendPacket(xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) override {
        switch (CommandId) {
            case Cmd_AuthByUserPassResp:
                return OnCmdAuthByUserPassResp(CommandId, RequestId, PayloadPtr, PayloadSize);

            default:
                X_DEBUG_PRINTF("unsupported protocol command");
                break;
        }
        return true;
    }

    bool OnCmdAuthByUserPassResp(xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) {
        auto P = xPPB_AuthByUserPassResp();
        if (!P.Deserialize(PayloadPtr, PayloadSize)) {
            X_DEBUG_PRINTF("invalid protocol");
            return false;
        }
        X_DEBUG_PRINTF("%s", P.ToString().c_str());

        return true;
    }

    //
};

//////////

auto IC  = xIoContext();
auto ICG = xResourceGuard(IC);

auto TA = xNetAddress::Parse("45.202.204.29:20005");

auto C  = xAuthTest();
auto CG = xResourceGuard(C, &IC, 200);

using namespace std::chrono_literals;

static void Test() {
    static xTimer TestTimer;
    if (TestTimer.TestAndTag(1s)) {
        auto T     = xPPB_AuthByUserPass();
        T.UserPass = "C_he_0_US_1001__5_78758832:1234567";
        T.ClientIp = xNetAddress::Parse("45.202.204.29:7777");
        C.PostMessage(Cmd_AuthByUserPass, 0, T);
    }
}

int main(int, char **) {

    auto I1 = C.AddServer(TA, "apitest", "123456");
    auto I2 = C.AddServer(TA, "apitest", "123456");
    auto I3 = C.AddServer(TA, "apitest", "123456");

    X_DEBUG_PRINTF("%" PRIx64 "", I1);
    X_DEBUG_PRINTF("%" PRIx64 "", I2);
    X_DEBUG_PRINTF("%" PRIx64 "", I3);

    while (true) {
        IC.LoopOnce();
        C.Tick();
        Test();
    }

    return 0;
    //
}
