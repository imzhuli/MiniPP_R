#include "./cache_manager.hpp"

#include "./service_common.hpp"

bool xCacheManager::Init(size_t MaxCacheNodeSize, size_t RequestPoolSize) {
    if (!CacheNodePool.Init(MaxCacheNodeSize)) {
        return false;
    }
    if (CacheRequestPool.Init(RequestPoolSize)) {
        CacheNodePool.Clean();
        return false;
    }
    return true;
}

void xCacheManager::Clean() {
    CacheRequestPool.Clean();
    CacheNodePool.Clean();
}

void xCacheManager::PostAcquireCacheNodeRequest(const std::string & Key, const xCacheRequestContext & Context) {
    auto NP   = (xCacheNode *)nullptr;
    auto Iter = CacheMap.find(Key);
    if (Iter == CacheMap.end()) {
        auto CNID = CacheNodePool.Acquire();
        if (!CNID) {
            ++LocalAudit.CacheLimit;
            return;
        }
        NP              = &CacheNodePool[CNID];
        NP->CacheNodeId = CNID;
        NP->Key         = Key;
        CacheMap[Key]   = NP;

        NP->TimestampMS = ServiceTicker();
        CacheQuickReleaseTimeoutList.AddTail(*NP);

        ++LocalAudit.CacheNew;
    }

    if (NP->State == xCacheNode::CACHE_STATE_RESULT) {
        OnResultData(Context, NP->DataPtr);
        ++LocalAudit.CacheHit;
        return;
    }

    auto RCtx = CacheRequestPool.Create();
    if (!RCtx) {
        ++LocalAudit.CacheQueryLimit;
        OnResultData(Context, nullptr);
        return;
    }
    RCtx->Context = Context;
    NP->PendingRequestList.AddTail(*RCtx);

    if (NP->State == xCacheNode::CACHE_STATE_QUERY) {
        ++LocalAudit.CacheQueryCombined;
        return;
    }

    assert(NP->State == xCacheNode::CACHE_STATE_INIT);
    if (!MakeAsyncQuery(NP->CacheNodeId, Key)) {
        ++LocalAudit.CacheQueryFailed;
        SetResultData(NP->CacheNodeId, nullptr);
        return;
    }
    ++LocalAudit.CacheQuery;
}

void xCacheManager::SetResultData(uint64_t CacheNodeId, const void * Data) {
    auto NP = CacheNodePool.CheckAndGet(CacheNodeId);
    if (!NP) {
        ++LocalAudit.DanglingResult;
        return;
    }

    if (auto ODP = Steal(NP->DataPtr)) {
        OnReleaseData(NP->CacheNodeId, ODP);
    }

    NP->DataPtr = Data;
    NP->State   = xCacheNode::CACHE_STATE_RESULT;
    if (Data) {
        ++LocalAudit.ValidResult;
    } else {
        ++LocalAudit.BadResult;
    }

    while (auto RCP = NP->PendingRequestList.PopHead()) {
        OnResultData(RCP->Context, NP->DataPtr);
        CacheRequestPool.Destroy(RCP);
    }
}

void xCacheManager::ReleaseCacheNode(xCacheNode * NP) {
    assert(NP && NP == CacheNodePool.CheckAndGet(NP->CacheNodeId));
    while (auto RCP = NP->PendingRequestList.PopHead()) {
        ++LocalAudit.CacheQueryCancelled;
        OnResultData(RCP->Context, nullptr);
        CacheRequestPool.Destroy(RCP);
    }
    if (NP->DataPtr) {
        OnReleaseData(NP->CacheNodeId, NP->DataPtr);
    }
}
