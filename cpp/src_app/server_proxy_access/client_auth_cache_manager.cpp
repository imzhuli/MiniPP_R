#include "./client_auth_cache_manager.hpp"

#include "./_global.hpp"

bool xPA_AuthCacheManager::Init(xIoContext * ICP, const xNetAddress & CCAddress, size_t CachePoolSize) {
    RuntimeAssert(RequestPool.Init(REQUEST_POOL_SIZE));
    RuntimeAssert(AuthCacheNodePool.Init(CachePoolSize));

    ReconfirmTimePointMS = Ticker() - AUTH_CACHE_RECONFIRM_TIMEOUT_MS;
    return true;
}

void xPA_AuthCacheManager::Clean() {
    Reset(AuthMap);
    AuthCacheNodePool.Clean();
    RequestPool.Clean();
}

void xPA_AuthCacheManager::Tick() {
    Tick(GetTimestampMS());
}
void xPA_AuthCacheManager::Tick(uint64_t NowMS) {
    Ticker.Update(NowMS);
    OnTick();
}

void xPA_AuthCacheManager::OnTick() {
    ReconfirmTimePointMS = Ticker() - AUTH_CACHE_RECONFIRM_TIMEOUT_MS;

    // remove timeout cache node:
    while (auto P = static_cast<xPA_AuthCacheNode *>(AuthCacheUpdateList.PopHead([KP = Ticker() - AUTH_CACHE_REQUEST_TIMEOUT_MS](const xPA_AuthCacheUpdateNode & N) {
               return N.LastUpdateTimestampMS < KP;
           }))) {
        DispatchResultList.GrabListTail(P->RequestList);
        ReleaseCacheNode(P);
    }
    while (auto P = static_cast<xPA_AuthCacheNode *>(AuthCacheLRUList.PopHead([KP = Ticker() - AUTH_CACHE_NODE_TIMEOUT_MS](const xPA_AuthCacheLRUNode & N) {
               return N.LastUsedTimestampMS < KP;
           }))) {
        assert(P->RequestList.IsEmpty());
        ReleaseCacheNode(P);
    }

    // dispatch results:
    while (auto P = static_cast<xPA_AuthRequestNode *>(DispatchResultList.PopHead())) {
        auto CNP = AuthCacheNodePool.CheckAndGet(P->CacheNodeId);
        GlobalClientConnectionManager.OnAuthResult(P->RequestSourceId, CNP ? &CNP->Result : nullptr);
        RequestPool.Release(P->RequestId);
        --GlobalLocalAudit.OngoingAuthRequestNumber;
    }
}

xPA_AuthCacheNode * xPA_AuthCacheManager::AcquireCacheNode(const std::string & UserPass) {
    auto Id = AuthCacheNodePool.Acquire();
    if (!Id) {
        return nullptr;
    }
    ++GlobalLocalAudit.AuthCacheNodeCount;

    auto NP         = &AuthCacheNodePool[Id];
    NP->UserPass    = UserPass;
    NP->CacheNodeId = Id;

    auto & Slot = AuthMap[UserPass];
    assert(Slot == nullptr);
    Slot = NP;
    return NP;
}

void xPA_AuthCacheManager::ReleaseCacheNode(xPA_AuthCacheNode * P) {
    auto Iter = AuthMap.find(P->UserPass);
    assert(Iter != AuthMap.end());
    assert(Iter->second == P);
    AuthMap.erase(Iter);

    assert(P == AuthCacheNodePool.CheckAndGet(P->CacheNodeId));
    AuthCacheNodePool.Release(P->CacheNodeId);
    --GlobalLocalAudit.AuthCacheNodeCount;
}

bool xPA_AuthCacheManager::RequestAuth(uint64_t RequestSourceId, const std::string & UserPass) {
    X_DEBUG_PRINTF("");
    auto Rid = RequestPool.Acquire();
    if (!Rid) {
        return false;
    }
    auto & Request           = RequestPool[Rid];
    Request.RequestId        = Rid;
    Request.RequestTimestamp = Ticker();
    Request.RequestSourceId  = RequestSourceId;
    ++GlobalLocalAudit.OngoingAuthRequestNumber;

    // try find local result:
    auto CNP  = static_cast<xPA_AuthCacheNode *>(nullptr);
    auto Iter = AuthMap.find(UserPass);
    if (Iter == AuthMap.end()) {
        CNP = AcquireCacheNode(UserPass);
        if (!CNP) {
            X_PERROR("No more cache node careted");
            DispatchResultList.AddTail(Request);
            return true;
        }
    } else {
        CNP = Iter->second;
    }

    Request.CacheNodeId = CNP->CacheNodeId;
    if (CNP->Result.Ready) {
        DispatchResultList.AddTail(Request);
    } else {
        CNP->RequestList.AddTail(Request);
    }

    if (CNP->LastUpdateTimestampMS <= ReconfirmTimePointMS) {
        PerformAuthRequest(CNP, UserPass);
    }
    HitCache(CNP);
    return true;
}

bool xPA_AuthCacheManager::RequestAuth(const xNetAddress & Address) {
    X_DEBUG_PRINTF("");
    return false;
}

void xPA_AuthCacheManager::HitCache(xPA_AuthCacheNode * CNP) {
    CNP->LastUsedTimestampMS = Ticker();
    AuthCacheLRUList.GrabTail(*CNP);
}

void xPA_AuthCacheManager::PerformAuthRequest(xPA_AuthCacheNode * CNP, const std::string & UserPass) {
    X_DEBUG_PRINTF("");
    // TODO: Post Request

    CNP->LastUpdateTimestampMS = Ticker();
    AuthCacheUpdateList.GrabTail(*CNP);
}
