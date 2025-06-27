#pragma once
#include "../lib_backend_connection/backend_connection_pool.hpp"
#include "../lib_server_util/auth_cache/auth_cache_client.hpp"
#include "../lib_server_util/base.hpp"

#include <pp_common/base.hpp>

struct xAC_AuthBackendConnectionPool : public xBackendConnectionPool {

    using xBackendConnectionPool::Clean;
    using xBackendConnectionPool::Init;

    bool OnBackendPacket(xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) override;
    bool OnCmdAuthByUserPassResp(xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize);

    //
};

struct xAC_AuthService : xService {

    bool Init(xIoContext * ICP, const xNetAddress & BindAddress);
    void Clean();
    void OnTick(uint64_t NowMS) override;

    bool OnClientPacket(xServiceClientConnection & Connection, xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) override;

    void PostResposne(xServiceClientConnection & Connection, xPacketRequestId RequestId, const xAuthCacheInfo * CacheNode);

    //
    xAC_AuthBackendConnectionPool BackendPool;
    xAuthCacheManager             CacheManager;
    xServiceRequestContextPool    RequestContextPool;
};
