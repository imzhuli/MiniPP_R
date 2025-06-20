#include "./auth_cache.hpp"

#include "./_global.hpp"

bool xAC_CacheManager::Init() {
    return true;
}

void xAC_CacheManager::Clean() {
    while (auto NP = (xAC_CacheNode *)TimeoutList.PopHead()) {
        delete NP;
    }
    Reset(CacheMap);
}

void xAC_CacheManager::Tick() {
    RemoveTimeoutCacheNodes();
}

std::string xAC_CacheManager::MakeCacheNodeKey(const std::string & Account, const std::string & Password) {
    return Account;
}

xAC_CacheNode * xAC_CacheManager::GetCacheNode(const std::string & Key) {
    auto Iter = CacheMap.find(Key);
    if (Iter == CacheMap.end()) {
        return nullptr;
    }
    return Iter->second;
}

bool xAC_CacheManager::UpdateCacheNode(const std::string & Key, xAC_CacheData && DataSource) {
    auto & NP = CacheMap[Key];
    if (!NP) {
        NP                  = new xAC_CacheNode();
        NP->UpdateTimestamp = ServiceTicker();
        NP->Key             = Key;
    }
    NP->Data = std::move(DataSource);
    return true;
}

void xAC_CacheManager::RemoveCacheNode(xAC_CacheNode * NP) {
    auto Iter = CacheMap.find(NP->Key);
    assert(Iter != CacheMap.end() && Iter->second == NP);
    CacheMap.erase(Iter);
    delete NP;
}

void xAC_CacheManager::RemoveTimeoutCacheNodes() {
    uint64_t KillTimepoint = ServiceTicker() - CacheNodeTimeoutMS;
    while (auto NP = (xAC_CacheNode *)TimeoutList.PopHead([KillTimepoint](const xAC_CacheTimeoutNode & N) { return N.UpdateTimestamp <= KillTimepoint; })) {
        RemoveCacheNode(NP);
    }
}