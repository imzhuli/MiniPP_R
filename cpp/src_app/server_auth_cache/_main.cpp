#include "../lib_backend_connection/backend_connection.hpp"

#include <pp_protocol/_backend/auth_by_user_pass.hpp>
#include <pp_protocol/command.hpp>

auto IC  = xIoContext();
auto ICG = xResourceGuard(IC);

auto TA = xNetAddress::Parse("45.202.204.29:20005");

struct xAuthTest : public xBackendConnection {

    using xBackendConnection::Clean;
    using xBackendConnection::Init;

    bool OnConnectionReady() override {
        X_DEBUG_PRINTF("");

        auto T     = xPPB_AuthByUserPass();
        T.UserPass = "C_he_0_US_1001__5_78758832:1234567";
        return PostMessage(Cmd_AuthByUserPass, 0, T);
    }

    //
};

int main(int, char **) {

    auto C  = xAuthTest();
    auto CG = xResourceGuard(C, &IC, TA, "apitest", "123456");

    while (true) {
        IC.LoopOnce();
        C.Tick();
    }

    return 0;
    //
}
