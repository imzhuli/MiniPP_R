#pragma once
#include <pp_common/base.hpp>

class xCacheNodeBase;
class xCacheManager;

class xCacheRequestContext {
    xVariable Value   = {};
    xVariable ValueEx = {};
};
class xCacheRequestContextNode : public xListNode {
    friend class xCacheManager;
    xCacheRequestContext Context;
};
using xCacheUpdateRequestList = xList<xCacheRequestContextNode>;

class xCacheNode final : xListNode {
    friend class xList<xCacheNode>;
    friend class xCacheManager;

    enum eState : uint16_t {
        CACHE_STATE_INIT   = 0x00,
        CACHE_STATE_QUERY  = 0x01,
        CACHE_STATE_RESULT = 0x02,
    };

    uint64_t                CacheNodeId;
    uint64_t                TimestampMS;
    eState                  State = CACHE_STATE_INIT;
    std::string             Key;
    xCacheUpdateRequestList PendingRequestList;

    const void * DataPtr = nullptr;
};
using xCacheTimeoutList = xList<xCacheNode>;

class xCacheManager {
public:
    struct xLocalAudit {
        size_t CacheNew            = 0;
        size_t CacheLimit          = 0;
        size_t CacheDelete         = 0;
        size_t CacheHit            = 0;
        size_t CacheQuery          = 0;
        size_t CacheQueryLimit     = 0;
        size_t CacheQueryFailed    = 0;
        size_t CacheQueryCombined  = 0;
        size_t CacheQueryTimeout   = 0;
        size_t CacheQueryCancelled = 0;

        size_t DanglingResult = 0;
        size_t ValidResult    = 0;
        size_t BadResult      = 0;
    };

    bool Init(size_t MaxCacheNodeCount = 1024, size_t RequestPoolSize = 1024);
    void Clean();

public:
    void PostAcquireCacheNodeRequest(const std::string & Key, const xCacheRequestContext & Context);

protected:
    void SetResultData(uint64_t CacheNodeId, const void * Data);

    virtual bool MakeAsyncQuery(uint64_t CacheNodeId, const std::string & Key)         = 0;
    virtual void OnResultData(const xCacheRequestContext & Context, const void * Data) = 0;
    virtual void OnReleaseData(uint64_t CacheNodeId, const void * Data)                = 0;

    auto GetAndResetLocalAudit() { return Steal(LocalAudit); }

private:
    void ReleaseCacheNode(xCacheNode * NP);

private:
    xIndexedStorage<xCacheNode>                   CacheNodePool;
    xMemoryPool<xCacheRequestContextNode>         CacheRequestPool;
    std::unordered_map<std::string, xCacheNode *> CacheMap;
    xCacheTimeoutList                             CacheTimeoutList;
    xCacheTimeoutList                             CacheQuickReleaseTimeoutList;
    xLocalAudit                                   LocalAudit;

    uint64_t CacheTimeoutMS             = 15 * 60'000;
    uint64_t CacheQuickReleaseTimeoutMS = 30'000;
};
