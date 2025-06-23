#pragma once
#include "./_mindef.hpp"

#include <map>
#include <pp_common/base.hpp>
#include <unordered_map>

struct xPA_AuthRequestNode : xListNode {
    uint64_t RequestId        = 0;
    uint64_t RequestTimestamp = 0;
    uint64_t RequestSourceId  = 0;
    uint64_t CacheNodeId      = 0;
};

struct xPA_AuthCacheUpdateNode : xListNode {
    uint64_t LastUpdateTimestampMS = 0;
};

struct xPA_AuthCacheLRUNode : xListNode {
    uint64_t LastUsedTimestampMS = 0;
};

struct xPA_AuthCacheNode
    : xPA_AuthCacheLRUNode
    , xPA_AuthCacheUpdateNode {
    xIndexId                   CacheNodeId = {};
    std::string                UserPass    = {};
    xPA_AuthResult             Result      = {};
    xList<xPA_AuthRequestNode> RequestList = {};
};

class xPA_AuthCacheManager : public xClientPool {
public:
    bool Init(xIoContext * ICP, const xNetAddress & CacheServerAddress, size_t CachePoolSize = DEFAULT_AUTH_CACHE_POOL_SIZE);
    void Clean();

    bool RequestAuth(uint64_t RequestSourceId, const std::string & UserPass);
    bool RequestAuth(const xNetAddress & Address);

protected:
    void OnTick(uint64_t NowMS) override;
    void HitCache(xPA_AuthCacheNode * CNP);
    void PerformAuthRequest(xPA_AuthCacheNode * CNP, const std::string & UserPass);
    auto AcquireCacheNode(const std::string & UserPass) -> xPA_AuthCacheNode *;
    void ReleaseCacheNode(xPA_AuthCacheNode * P);

    bool OnServerPacket(xClientConnection & CC, xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) override;

private:
    xTicker  Ticker;
    uint64_t ReconfirmTimePointMS = 0;

    xIndexedStorage<xPA_AuthRequestNode>                 RequestPool;
    xIndexedStorage<xPA_AuthCacheNode>                   AuthCacheNodePool;
    std::unordered_map<std::string, xPA_AuthCacheNode *> AuthMap;  // UserPass -> CacheNodeIndex

    xList<xPA_AuthCacheLRUNode>    AuthCacheLRUList;
    xList<xPA_AuthCacheUpdateNode> AuthCacheUpdateList;

    xList<xPA_AuthRequestNode> DispatchResultList;
};
