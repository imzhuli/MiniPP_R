#pragma once
#include "../extra/auth_cache_server_list_client.hpp"
#include "../service_client_hash_pool.hpp"
#include "./auth_cache_info.hpp"

#include <pp_common/base.hpp>
#include <pp_protocol/internal/all.hpp>
#include <unordered_map>

class xAuthCacheServerListUpdater;
class xAuthCacheClient;
class xAuthCacheRequester;

class xAuthCacheServerListUpdater : public xDownloadAuthCacheServerListClient {
protected:
    void OnServerListUpdated() override;

private:
    xAuthCacheClient * Owner = nullptr;
};

class xAuthCacheRequester : public xServiceClientHashPool {
private:
    xAuthCacheClient * Owner = nullptr;
};

class xAuthCacheClient {
    friend class xAuthCacheServerListUpdater;
    friend class xAuthCacheRequester;
};
