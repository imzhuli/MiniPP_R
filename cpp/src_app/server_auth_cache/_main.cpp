#include "../lib_backend_connection/backend_connection_pool.hpp"
#include "./_global.hpp"
#include "./auth_cache.hpp"

#include <pp_protocol/_backend/auth_by_user_pass.hpp>
#include <pp_protocol/command.hpp>

struct xAuthTest : public xBackendConnectionPool {

    using xBackendConnectionPool::Clean;
    using xBackendConnectionPool::Init;

    bool OnBackendPacket(xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) override {
        switch (CommandId) {
            case Cmd_BackendAuthByUserPassResp:
                return OnCmdAuthByUserPassResp(CommandId, RequestId, PayloadPtr, PayloadSize);

            default:
                X_DEBUG_PRINTF("unsupported protocol command");
                break;
        }
        return true;
    }

    bool OnCmdAuthByUserPassResp(xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) {
        auto P = xPPB_BackendAuthByUserPassResp();
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
        auto T     = xPPB_BackendAuthByUserPass();
        T.UserPass = "C_he_0_US_1001__5_78758832:1234567";
        T.ClientIp = xNetAddress::Parse("45.202.204.29:7777");
        C.PostMessage(Cmd_BackendAuthByUserPass, 0, T);
    }
}

int main(int argc, char ** argv) {
    auto Env = xRuntimeEnvGuard(argc, argv);
    auto CL  = xConfigLoader(RuntimeEnv.DefaultConfigFilePath);
    CL.Require(BindAddress, "ServerIdCenterAddress");
    CL.Require(ServerIdCenterAddress, "ServerIdCenterAddress");
    CL.Require(ServerListRegisterAddress, "ServerListRegisterAddress");
    CL.Require(ExportServerAddress, "ExportServerAddress");

    auto ACG   = xResourceGuard(AuthService, &IC, BindAddress);
    auto SICG  = xResourceGuard(ServerIdClient, &IC, ServerIdCenterAddress);
    auto SLRAG = xResourceGuard(RegisterServerClient, &IC, ServerListRegisterAddress);

    RuntimeAssert(SICG);
    RuntimeAssert(SLRAG);

    Touch(Test);
    Touch(C);

    while (true) {
        ServiceTicker.Update();
        IC.LoopOnce();
        TickAll(ServiceTicker(), AuthService, ServerIdClient, RegisterServerClient);
    }

    return 0;
    //
}
