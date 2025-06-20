#include "./auth_service.hpp"

#include <pp_protocol/_backend/auth_by_user_pass.hpp>
#include <pp_protocol/command.hpp>
#include <pp_protocol/internal/all.hpp>

bool xAC_AuthBackendConnectionPool::OnBackendPacket(xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) {
    switch (CommandId) {
        case Cmd_AuthByUserPassResp:
            return OnCmdAuthByUserPassResp(CommandId, RequestId, PayloadPtr, PayloadSize);

        default:
            X_DEBUG_PRINTF("unsupported protocol command");
            break;
    }
    return true;
}

bool xAC_AuthBackendConnectionPool::OnCmdAuthByUserPassResp(xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) {
    auto P = xPPB_AuthByUserPassResp();
    if (!P.Deserialize(PayloadPtr, PayloadSize)) {
        X_DEBUG_PRINTF("invalid protocol");
        return false;
    }
    X_DEBUG_PRINTF("%s", P.ToString().c_str());

    return true;
}

/////////////////////////////

bool xAC_AuthService::Init() {
    RuntimeAssert(CacheManager.Init());
    RuntimeAssert(RequestContextPool.Init(20'000));
    return true;
}
void xAC_AuthService::Clean() {
    RequestContextPool.Clean();
    CacheManager.Clean();
}

bool xAC_AuthService::OnClientPacket(xServiceClientConnection & Connection, xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) {

    if (CommandId != Cmd_AuthService_QueryAuthCache) {
        X_DEBUG_PRINTF("Invalid command id");
        return true;
    }

    auto Req = xQueryAuthCache();
    if (!Req.Deserialize(PayloadPtr, PayloadSize)) {
        X_DEBUG_PRINTF("Invalid request");
        return true;
    }

    auto Resp = xQueryAuthCacheResp();
    PostMessage(Connection, Cmd_AuthService_QueryAuthCacheResp, RequestId, Resp);
    return true;
}
