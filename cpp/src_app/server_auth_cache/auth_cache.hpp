#pragma once
#include <pp_common/base.hpp>
#include <unordered_map>

struct xAC_CacheTimeoutNode : xListNode {
    uint64_t UpdateTimestamp = {};
};
using xAC_CacheTimeoutList = xList<xAC_CacheTimeoutNode>;

struct xAC_CacheData {
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

struct xAC_CacheNode : xAC_CacheTimeoutNode {
    std::string   Key;
    xAC_CacheData Data;
};

class xAC_CacheManager {
public:
    bool Init();
    void Clean();
    void Tick();

    std::string     MakeCacheNodeKey(const std::string & Account, const std::string & Password);
    xAC_CacheNode * GetCacheNode(const std::string & Key);
    bool            UpdateCacheNode(const std::string & Key, xAC_CacheData && DataSource);

private:
    void RemoveCacheNode(xAC_CacheNode * NP);
    void RemoveTimeoutCacheNodes();

private:
    std::unordered_map<std::string, xAC_CacheNode *> CacheMap;
    xAC_CacheTimeoutList                             TimeoutList;
};
