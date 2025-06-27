#include "./auth_cache_info.hpp"

#include "../service_common.hpp"

auto xAuthCacheManager::AcquireCacheNode(const std::string & Key) -> xAuthCacheNode * {
    auto Iter = CacheMap.find(Key);
    if (Iter == CacheMap.end()) {
        ++LocalAudit.CacheMiss;
        auto NP = CacheNodePool.Create();
        if (!NP) {
            ++LocalAudit.CacheLimit;
            return nullptr;
        }
        CacheMap[Key]       = NP;
        NP->Key             = Key;
        NP->UpdateTimestamp = ServiceTicker();
        CacheTimeoutList.AddTail(*NP);
    }
    return Iter->second;
}

xIndexId xAuthCacheManager::UpdateCacheNode(const std::string & Key, const xNone &) {
    auto Iter = CacheMap.find(Key);
    if (Iter == CacheMap.end()) {
        ++LocalAudit.DanglingResult;
        return {};
    }
    ++LocalAudit.CacheUpdate;
    auto NP = Iter->second;
    Reset(NP->InfoOpt);
    Reset(NP->UpdateTimestamp, ServiceTicker());
    CacheTimeoutList.GrabTail(*NP);

    return Steal(NP->PendingRequestId);
}

xIndexId xAuthCacheManager::UpdateCacheNode(const std::string & Key, xAuthCacheInfo && DataSource) {
    auto Iter = CacheMap.find(Key);
    if (Iter == CacheMap.end()) {
        ++LocalAudit.DanglingResult;
        return {};
    }
    ++LocalAudit.CacheUpdate;
    auto NP = Iter->second;
    Reset(NP->InfoOpt, std::move(DataSource));
    Reset(NP->UpdateTimestamp, ServiceTicker());
    CacheTimeoutList.GrabTail(*NP);

    return Steal(NP->PendingRequestId);
}

void xAuthCacheManager::BindRequestId(xAuthCacheNode * NP, xIndexId RequestId) {
    assert(!NP->PendingRequestId);
    NP->PendingRequestId = RequestId;
    NP->UpdateTimestamp  = ServiceTicker();
    CacheRequestTimeoutList.GrabTail(*NP);
}

void xAuthCacheManager::AppendRequestNode(xAuthCacheNode * NP, xAuthRequestNode & RN) {
    assert(NP->PendingRequestId);
    assert(!xListNode::IsLinked(RN));
    NP->PendingRequests.AddTail(RN);
}

void xAuthCacheManager::RemoveCacheNode(xAuthCacheNode * NP) {
    auto Iter = CacheMap.find(NP->Key);
    assert(Iter != CacheMap.end() && Iter->second == NP);
    CacheMap.erase(Iter);
    while (NP->PendingRequests.PopHead()) {
        Pass();
    }
    CacheNodePool.Destroy(NP);
}

void xAuthCacheManager::RemoveTimeoutCacheNodes(uint64_t TimeoutMS) {
    uint64_t KillTimepoint = ServiceTicker() - TimeoutMS;
    while (auto NP = (xAuthCacheNode *)CacheTimeoutList.PopHead([KillTimepoint](const xAuthCacheTimeoutNode & N) { return N.UpdateTimestamp <= KillTimepoint; })) {
        RemoveCacheNode(NP);
    }
}

void xAuthCacheManager::RemoveTimeoutCacheUpdateNodes(uint64_t TimeoutMS) {
    // uint64_t KillTimepoint = ServiceTicker() - TimeoutMS;
    // while (auto NP = (xAuthCacheNode *)CacheUpdateTimeoutList.PopHead([KillTimepoint](const xAuthCacheTimeoutNode & N) { return N.UpdateTimestamp <= KillTimepoint; })) {
    //     RemoveCacheNode(NP);
    // }
}
