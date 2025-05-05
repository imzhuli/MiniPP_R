#include "./auth_cache_server_manager.hpp"

#include <pp_protocol/ac_cc/alloc_cache_server_id.hpp>
#include <pp_protocol/command.hpp>

bool xCC_AuthCacheServerManager::Init(xIoContext * IoContextPtr, const xNetAddress & BindAddress) {
    RuntimeAssert(xService::Init(IoContextPtr, BindAddress));
    RuntimeAssert(IdManager.Init());

    return true;
}

void xCC_AuthCacheServerManager::Clean() {
    IdManager.Clean();
    xService::Clean();
}

bool xCC_AuthCacheServerManager::OnClientPacket(
    xServiceClientConnection & Connection, xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize
) {
    switch (CommandId) {
        case Cmd_AC_CC_AllocServerId:
            return OnAllocAuthCacheServerId(Connection, RequestId, PayloadPtr, PayloadSize);
        case Cmd_AC_CC_DownloadDispatcherList:
            return OnAllocDownloadDispatcherList(Connection, RequestId, PayloadPtr, PayloadSize);
    }
    return false;
}

bool xCC_AuthCacheServerManager::OnAllocAuthCacheServerId(xServiceClientConnection & Connection, uint64_t RequestId, ubyte * PayloadPtr, size_t PayloadSize) {
    auto S = x_AC_CC_AllocServerId();
    if (!S.Deserialize(PayloadPtr, PayloadSize)) {
        return false;
    }
    auto & V = Connection.GetUserContext();
    if (V.U32) {
        X_DEBUG_PRINTF("already allocated server id");
        return false;
    }
    auto NewServerId = IdManager.Acquire();
    if (!NewServerId) {
        X_DEBUG_PRINTF("not enough server id");
        return false;
    }
    V.U32 = NewServerId;

    auto R     = x_AC_CC_AllocServerIdResp();
    R.ServerId = V.U32;
    PostMessage(Connection, Cmd_AC_CC_AllocServerIdResp, RequestId, R);
    return true;
}

bool xCC_AuthCacheServerManager::OnAllocDownloadDispatcherList(
    xServiceClientConnection & Connection, uint64_t RequestId, ubyte * PayloadPtr, size_t PayloadSize
) {

    return false;
}
