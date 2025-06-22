#include "./server_list_manager.hpp"

bool xSL_InternalServerListManager::AddAuthCacheServerInfo(uint64_t ServerId, xNetAddress ServerAddress) {
    auto It    = std::lower_bound(AuthCacheServerInfoList.begin(), AuthCacheServerInfoList.end(), ServerId, [](const auto & R, uint64_t Id) { return R->ServerId < Id; });
    auto Found = (It != AuthCacheServerInfoList.end()) && ((*It)->ServerId == ServerId);
    if (Found) {
        return false;
    }
    AuthCacheServerInfoList.emplace(
        It,
        new xSL_AuthCacheServerInfo{
            .ServerType    = AUTH_CACHE,
            .ServerId      = ServerId,
            .ServerAddress = ServerAddress,
        }
    );
    return true;
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
