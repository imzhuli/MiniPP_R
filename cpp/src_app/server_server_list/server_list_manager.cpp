#include "./server_list_manager.hpp"

auto xSL_InternalServerListManager::AddAuthCacheServerInfo(uint64_t ServerId, xNetAddress ServerAddress) -> xSL_AuthCacheServerInfo * {
    if (AuthCacheServerInfoList.size() >= MaxAuthCacheServerCount) {
        return nullptr;
    }

    auto It    = std::lower_bound(AuthCacheServerInfoList.begin(), AuthCacheServerInfoList.end(), ServerId, [](const auto & R, uint64_t Id) { return R->ServerId < Id; });
    auto Found = (It != AuthCacheServerInfoList.end()) && ((*It)->ServerId == ServerId);
    if (Found) {
        return nullptr;
    }
    It = AuthCacheServerInfoList.emplace(
        It,
        new xSL_AuthCacheServerInfo{
            .ServerType    = eServerType::AUTH_CACHE,
            .ServerId      = ServerId,
            .ServerAddress = ServerAddress,
        }
    );
    return (*It).get();
}

void xSL_InternalServerListManager::RemoveAuthCacheServerInfo(uint64_t ServerId) {
    auto It = std::lower_bound(AuthCacheServerInfoList.begin(), AuthCacheServerInfoList.end(), ServerId, [](const auto & R, uint64_t Id) { return R->ServerId < Id; });
    if (It == AuthCacheServerInfoList.end()) {
        return;
    }
    if ((*It)->ServerId != ServerId) {
        return;
    }
    AuthCacheServerInfoList.erase(It);
}

auto xSL_InternalServerListManager::GetAuthCacheServerInfo(uint64_t ServerId) -> const xSL_AuthCacheServerInfo * {
    auto It = std::lower_bound(AuthCacheServerInfoList.begin(), AuthCacheServerInfoList.end(), ServerId, [](const auto & R, uint64_t Id) { return R->ServerId < Id; });
    if (It == AuthCacheServerInfoList.end()) {
        return nullptr;
    }
    if ((*It)->ServerId != ServerId) {
        return nullptr;
    }
    return (*It).get();
}

/////////////////////////////////////////////////