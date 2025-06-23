#include "./server_list_manager.hpp"

#include "./_global.hpp"

static constexpr const uint64_t UPGRADE_VERSION_TIMEOUT_MS = 1 * 60'000;

void xSL_InternalServerListManager::OnTick(uint64_t NowMS) {
    Ticker.Update(NowMS);

    if (AuthCacheServerInfoListDirty && (NowMS - AuthCacheServerInfoListVersionTimestampMS) > UPGRADE_VERSION_TIMEOUT_MS) {
        Logger->I("Update VersionedAuthCacheServerInfoList");
        VersionedAuthCacheServerInfoList          = AuthCacheServerInfoList;
        AuthCacheServerInfoListVersionTimestampMS = NowMS;
        Reset(AuthCacheServerInfoListDirty);
    }
}

auto xSL_InternalServerListManager::AddAuthCacheServerInfo(uint64_t ServerId, xNetAddress ServerAddress) -> xSL_AuthCacheServerInfo * {
    if (AuthCacheServerInfoList.size() >= MAX_AUTH_CACHE_SERVER_COUNT) {
        return nullptr;
    }

    auto It    = std::lower_bound(AuthCacheServerInfoList.begin(), AuthCacheServerInfoList.end(), ServerId, [](const auto & R, uint64_t Id) { return R.ServerId < Id; });
    auto Found = (It != AuthCacheServerInfoList.end()) && (It->ServerId == ServerId);
    if (Found) {
        return nullptr;
    }
    It = AuthCacheServerInfoList.emplace(
        It,
        xSL_AuthCacheServerInfo{
            .ServerType    = eServerType::AUTH_CACHE,
            .ServerId      = ServerId,
            .ServerAddress = ServerAddress,
        }
    );
    AuthCacheServerInfoListVersionTimestampMS = Ticker();
    AuthCacheServerInfoListDirty              = true;
    return &(*It);
}

void xSL_InternalServerListManager::RemoveAuthCacheServerInfo(uint64_t ServerId) {
    auto It = std::lower_bound(AuthCacheServerInfoList.begin(), AuthCacheServerInfoList.end(), ServerId, [](const auto & R, uint64_t Id) { return R.ServerId < Id; });
    if (It == AuthCacheServerInfoList.end()) {
        return;
    }
    if (It->ServerId != ServerId) {
        return;
    }
    AuthCacheServerInfoList.erase(It);
}

auto xSL_InternalServerListManager::GetAuthCacheServerInfo(uint64_t ServerId) -> const xSL_AuthCacheServerInfo * {
    auto It = std::lower_bound(AuthCacheServerInfoList.begin(), AuthCacheServerInfoList.end(), ServerId, [](const auto & R, uint64_t Id) { return R.ServerId < Id; });
    if (It == AuthCacheServerInfoList.end()) {
        return nullptr;
    }
    if (It->ServerId != ServerId) {
        return nullptr;
    }
    return &(*It);
}

/////////////////////////////////////////////////
