#include "./client_auth_cache_manager.hpp"

static constexpr const size_t REQUEST_POOL_SIZE = 5'0000;

static constexpr const uint64_t AUTH_REQUEST_TIMEOUT_MS    = 3'000;
static constexpr const uint64_t AUTH_CACHE_NODE_TIMEOUT_MS = 5 * 60'000;

bool xPA_AuthCacheManager::Init(xIoContext * ICP, const xNetAddress & CCAddress, size_t CachePoolSize) {
    RuntimeAssert(RequestPool.Init(REQUEST_POOL_SIZE));
    RuntimeAssert(AuthCachePool.Init(CachePoolSize));

    return true;
}

void xPA_AuthCacheManager::Clean() {
    AuthCachePool.Clean();
}

void xPA_AuthCacheManager::Tick() {
    Tick(GetTimestampMS());
}
void xPA_AuthCacheManager::Tick(uint64_t NowMS) {
    Ticker.Update(NowMS);
    OnTick();
}

void xPA_AuthCacheManager::OnTick() {
    // remove timeout cache node:
    while (auto P = static_cast<xPA_AuthCacheNode *>(OngoingRequestList.PopHead([KP = Ticker() - AUTH_REQUEST_TIMEOUT_MS](const xPA_AuthRequestNode & N) {
               return N.RequestStartTimestamp < KP;
           }))) {
        ReleaseCacheNode(P);
    }

    while (auto P = static_cast<xPA_AuthCacheNode *>(
               AuthCacheTimeoutList.PopHead([KP = Ticker() - AUTH_CACHE_NODE_TIMEOUT_MS](const xPA_AuthCacheTimeoutNode & N) { return N.TimestampMS < KP; })
           )) {
        ReleaseCacheNode(P);
    }

    while (auto P = static_cast<xPA_AuthCacheNode *>(
               AuthCacheReConfirmList.PopHead([KP = Ticker() - AUTH_REQUEST_TIMEOUT_MS * 2](const xPA_AuthCacheTimeoutNode & N) { return N.TimestampMS < KP; })
           )) {
        ReleaseCacheNode(P);
    }
}

xPA_AuthCacheNode * xPA_AuthCacheManager::AcquireCacheNode() {
    auto Id = AuthCachePool.Acquire();
    if (!Id) {
        return nullptr;
    }
    auto & N = AuthCachePool[Id];
    N.NodeId = Id;
    return &N;
}

void xPA_AuthCacheManager::ReleaseCacheNode(xPA_AuthCacheNode * P) {
    assert(P == AuthCachePool.CheckAndGet(P->NodeId));

    AuthCachePool.Release(P->NodeId);
}