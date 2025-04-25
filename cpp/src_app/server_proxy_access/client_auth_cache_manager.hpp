#pragma once
#include <map>
#include <pp_common/base.hpp>
#include <unordered_map>

struct xPA_AuthRequestNode : xListNode {
    uint64_t RequestStartTimestamp = 0;
    xIndexId AuthCacheNodeId       = 0;
};

struct xPA_AuthCacheLRUNode : xListNode {};

struct xPA_AuthCacheTimeoutNode : xListNode {
    uint64_t TimestampMS = 0;
};

struct xPA_AuthCacheNode
    : xPA_AuthCacheTimeoutNode
    , xPA_AuthRequestNode
    , xPA_AuthCacheLRUNode {
    xIndexId                   NodeId;
    std::string                AuthString;
    uint64_t                   AcquireRequestId          = 0;
    bool                       IsIpAccount               = false;
    bool                       BlockedAccount            = false;
    bool                       ChangeDevicePerConnection = false;
    uint64_t                   RelayServerRuntimeId;
    uint64_t                   DeviceRuntimeId;
    xList<xPA_AuthRequestNode> PendingRequests;
};

class xPA_AuthCacheManager {
public:
    bool Init(xIoContext * ICP, const xNetAddress & CCAddress, size_t CachePoolSize = 10'0000);
    void Clean();
    void Tick();
    void Tick(uint64_t NowMS);

protected:
    void OnTick();
    void UpdateUsage(xPA_AuthCacheNode & Node) {
        AuthCacheLRUList.GrabTail(Node);
    }
    void ReconfirmAccount(xPA_AuthCacheNode & Node) {
        // TODO: do request

        //
        Node.RequestStartTimestamp = Ticker();
        AuthCacheReConfirmList.GrabTail(Node);
    }
    auto AcquireCacheNode() -> xPA_AuthCacheNode *;
    void ReleaseCacheNode(xPA_AuthCacheNode * P);

private:
    xTicker                              Ticker;
    xIndexedStorage<xPA_AuthRequestNode> RequestPool;
    xIndexedStorage<xPA_AuthCacheNode>   AuthCachePool;
    xList<xPA_AuthRequestNode>           OngoingRequestList;
    xList<xPA_AuthCacheTimeoutNode>      AuthCacheTimeoutList;
    xList<xPA_AuthCacheTimeoutNode>      AuthCacheReConfirmList;
    xList<xPA_AuthCacheLRUNode>          AuthCacheLRUList;

    std::unordered_map<std::string, xIndexId> AuthMap;
    std::map<xNetAddress, xIndexId>           IpAuthMap;
};
