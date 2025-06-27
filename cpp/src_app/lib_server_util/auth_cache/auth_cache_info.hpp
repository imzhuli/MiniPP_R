#pragma once
#include <core/memory_pool.hpp>
#include <pp_common/base.hpp>

struct xAuthCacheInfo {
    uint64_t AuditId;

    xCountryId CountryId;
    xStateId   StateId;
    xCityId    CityId;

    bool IsBlocked;
    bool RequireIpv6;
    bool RequireUdp;
    bool RequireRemoteDns;
    bool AutoChangeIp;
    bool FixDeviceTimeout;

    std::string PAToken;
};

struct xAuthRequestNode : xListNode {
    xVariable Context   = {};
    xVariable ContextEx = {};
};
using xAuthRequestContextList = xList<xAuthRequestNode>;

struct xAuthCacheTimeoutNode : xListNode {
    uint64_t UpdateTimestamp = {};
};
using xAuthCacheTimeoutList = xList<xAuthCacheTimeoutNode>;

struct xAuthCacheNode : xAuthCacheTimeoutNode {

    using xAuthCacheFlags                             = uint16_t;
    static constexpr const xAuthCacheFlags NONE       = 0x00;
    static constexpr const xAuthCacheFlags QUERYING   = 0x01;
    static constexpr const xAuthCacheFlags HAS_RESULT = 0x02;
    static constexpr const xAuthCacheFlags BAD_RESULT = HAS_RESULT | 0x04;

    std::string             Key;
    xAuthCacheFlags         Flags = NONE;
    xIndexId                PendingRequestId;
    xAuthRequestContextList PendingRequests;
    xAuthCacheInfo          InfoOpt;
};

class xAuthCacheManager {
public:
    struct xLocalAudit {
        size_t CacheHit       = 0;
        size_t CacheMiss      = 0;
        size_t CacheLimit     = 0;
        size_t CacheUpdate    = 0;
        size_t TimeoutRequest = 0;
        size_t DanglingResult = 0;
    };

public:
    bool Init(size_t MaxCacheNodeCount) { return true; };
    void Clean() {};

    auto AcquireCacheNode(const std::string & Key) -> xAuthCacheNode *;
    auto RemoveTimeoutCacheNodes(uint64_t TimeoutMS) -> void;
    auto RemoveTimeoutCacheUpdateNodes(uint64_t TimeoutMS) -> void;

    auto GetAndResetLocalAudit() { return Steal(LocalAudit); }

private:
    auto UpdateCacheNode(const std::string & Key, const xNone &) -> xIndexId;
    auto UpdateCacheNode(const std::string & Key, xAuthCacheInfo && Info) -> xIndexId;
    void BindRequestId(xAuthCacheNode * NP, xIndexId RequestId);
    void AppendRequestNode(xAuthCacheNode * NP, xAuthRequestNode & RN);
    void RemoveCacheNode(xAuthCacheNode * NP);

private:
    xel::xMemoryPool<xAuthCacheNode>                  CacheNodePool;
    std::unordered_map<std::string, xAuthCacheNode *> CacheMap;
    xAuthCacheTimeoutList                             CacheTimeoutList;
    xAuthCacheTimeoutList                             CacheRequestTimeoutList;
    xLocalAudit                                       LocalAudit;
};
