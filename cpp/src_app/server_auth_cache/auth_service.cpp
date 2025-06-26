#include "./auth_service.hpp"

#include <pp_protocol/_backend/auth_by_user_pass.hpp>
#include <pp_protocol/command.hpp>
#include <pp_protocol/internal/all.hpp>

bool xAC_AuthBackendConnectionPool::OnBackendPacket(xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) {
    switch (CommandId) {
        case Cmd_BackendAuthByUserPassResp:
            return OnCmdAuthByUserPassResp(CommandId, RequestId, PayloadPtr, PayloadSize);

        default:
            X_DEBUG_PRINTF("unsupported protocol command");
            break;
    }
    return true;
}

bool xAC_AuthBackendConnectionPool::OnCmdAuthByUserPassResp(xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) {
    auto P = xPPB_BackendAuthByUserPassResp();
    if (!P.Deserialize(PayloadPtr, PayloadSize)) {
        X_DEBUG_PRINTF("invalid protocol");
        return false;
    }
    X_DEBUG_PRINTF("%s", P.ToString().c_str());

    return true;
}

/////////////////////////////

bool xAC_AuthService::Init(xIoContext * ICP, const xNetAddress & BindAddress) {
    RuntimeAssert(xService::Init(ICP, BindAddress, 10000, true));
    RuntimeAssert(CacheManager.Init());
    RuntimeAssert(RequestContextPool.Init(20'000));
    return true;
}

void xAC_AuthService::Clean() {
    RequestContextPool.Clean();
    CacheManager.Clean();
}

void xAC_AuthService::OnTick(uint64_t NowMS) {
    CacheManager.Tick();
    RequestContextPool.RemoveTimeoutRequests(2'000);
    TickAll(NowMS, BackendPool);
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

    auto NP = CacheManager.GetCacheNode(std::string(Req.UserPass));
    if (!NP) {
        PostResposne(Connection, RequestId, NP);
        return true;
    }

    Todo("make async request");
    return true;
}

void xAC_AuthService::PostResposne(xServiceClientConnection & Connection, xPacketRequestId RequestId, const xAC_CacheNode * NP) {
    auto   Resp  = xQueryAuthCacheResp();
    auto & C     = NP->Data;
    Resp.AuditId = C.AuditId;

    Resp.CountryId        = C.CountryId;
    Resp.StateId          = C.StateId;
    Resp.CityId           = C.CityId;
    Resp.IsBlocked        = C.IsBlocked;
    Resp.RequireIpv6      = C.RequireIpv6;
    Resp.RequireUdp       = C.RequireUdp;
    Resp.RequireRemoteDns = C.RequireRemoteDns;
    Resp.AutoChangeIp     = C.AutoChangeIp;
    Resp.PAToken          = C.PAToken;

    PostMessage(Connection, Cmd_AuthService_QueryAuthCacheResp, RequestId, Resp);
}