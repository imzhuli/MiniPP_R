#include "./server_list_manager.hpp"

#include "./_global.hpp"

static constexpr const uint64_t UPGRADE_VERSION_TIMEOUT_MS = 1 * 60'000;

void xSL_InternalServerListManager::OnTick(uint64_t NowMS) {
    Ticker.Update(NowMS);
    if (AuthCacheServerInfoListDirty && (NowMS - AuthCacheServerInfoListVersionTimestampMS) > UPGRADE_VERSION_TIMEOUT_MS) {
        Logger->I("Update VersionedAuthCacheServerInfoList");
        if (!++AuthCacheServerInfoListVersion) {
            ++AuthCacheServerInfoListVersion;
        }
        VersionedAuthCacheServerInfoList          = AuthCacheServerInfoList;
        AuthCacheServerInfoListVersionTimestampMS = NowMS;
        Reset(AuthCacheServerInfoListDirty);
    }

    if (AuditDeviceServerInfoListDirty && (NowMS - AuditDeviceServerInfoListVersionTimestampMS) > UPGRADE_VERSION_TIMEOUT_MS) {
        Logger->I("Update VersionedAuditDeviceServerInfoList");
        if (!++AuditDeviceServerInfoListVersion) {
            ++AuditDeviceServerInfoListVersion;
        }
        VersionedAuditDeviceServerInfoList          = AuditDeviceServerInfoList;
        AuditDeviceServerInfoListVersionTimestampMS = NowMS;
        Reset(AuditDeviceServerInfoListDirty);
    }

    if (AuditAccountServerInfoListDirty && (NowMS - AuditAccountServerInfoListVersionTimestampMS) > UPGRADE_VERSION_TIMEOUT_MS) {
        Logger->I("Update VersionedAuditAccountServerInfoList");
        if (!++AuditAccountServerInfoListVersion) {
            ++AuditAccountServerInfoListVersion;
        }
        VersionedAuditAccountServerInfoList          = AuditAccountServerInfoList;
        AuditAccountServerInfoListVersionTimestampMS = NowMS;
        Reset(AuditAccountServerInfoListDirty);
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

auto xSL_InternalServerListManager::AddAuditDeviceServerInfo(uint64_t ServerId, xNetAddress ServerAddress) -> xSL_AuditDeviceServerInfo * {
    if (AuditDeviceServerInfoList.size() >= MAX_AUTH_CACHE_SERVER_COUNT) {
        return nullptr;
    }

    auto It    = std::lower_bound(AuditDeviceServerInfoList.begin(), AuditDeviceServerInfoList.end(), ServerId, [](const auto & R, uint64_t Id) { return R.ServerId < Id; });
    auto Found = (It != AuditDeviceServerInfoList.end()) && (It->ServerId == ServerId);
    if (Found) {
        return nullptr;
    }
    It = AuditDeviceServerInfoList.emplace(
        It,
        xSL_AuditDeviceServerInfo{
            .ServerType    = eServerType::AUDIT_DEVICE_CACHE,
            .ServerId      = ServerId,
            .ServerAddress = ServerAddress,
        }
    );
    AuditDeviceServerInfoListVersionTimestampMS = Ticker();
    AuditDeviceServerInfoListDirty              = true;
    return &(*It);
}

void xSL_InternalServerListManager::RemoveAuditDeviceServerInfo(uint64_t ServerId) {
    auto It = std::lower_bound(AuditDeviceServerInfoList.begin(), AuditDeviceServerInfoList.end(), ServerId, [](const auto & R, uint64_t Id) { return R.ServerId < Id; });
    if (It == AuditDeviceServerInfoList.end()) {
        return;
    }
    if (It->ServerId != ServerId) {
        return;
    }
    AuditDeviceServerInfoList.erase(It);
}

auto xSL_InternalServerListManager::GetAuditDeviceServerInfo(uint64_t ServerId) -> const xSL_AuditDeviceServerInfo * {
    auto It = std::lower_bound(AuditDeviceServerInfoList.begin(), AuditDeviceServerInfoList.end(), ServerId, [](const auto & R, uint64_t Id) { return R.ServerId < Id; });
    if (It == AuditDeviceServerInfoList.end()) {
        return nullptr;
    }
    if (It->ServerId != ServerId) {
        return nullptr;
    }
    return &(*It);
}

//////////////////////////

auto xSL_InternalServerListManager::AddAuditAccountServerInfo(uint64_t ServerId, xNetAddress ServerAddress) -> xSL_AuditAccountServerInfo * {
    if (AuditAccountServerInfoList.size() >= MAX_AUTH_CACHE_SERVER_COUNT) {
        return nullptr;
    }

    auto It    = std::lower_bound(AuditAccountServerInfoList.begin(), AuditAccountServerInfoList.end(), ServerId, [](const auto & R, uint64_t Id) { return R.ServerId < Id; });
    auto Found = (It != AuditAccountServerInfoList.end()) && (It->ServerId == ServerId);
    if (Found) {
        return nullptr;
    }
    It = AuditAccountServerInfoList.emplace(
        It,
        xSL_AuditAccountServerInfo{
            .ServerType    = eServerType::AUDIT_ACCOUNT_CACHE,
            .ServerId      = ServerId,
            .ServerAddress = ServerAddress,
        }
    );
    AuditAccountServerInfoListVersionTimestampMS = Ticker();
    AuditAccountServerInfoListDirty              = true;
    return &(*It);
}

void xSL_InternalServerListManager::RemoveAuditAccountServerInfo(uint64_t ServerId) {
    auto It = std::lower_bound(AuditAccountServerInfoList.begin(), AuditAccountServerInfoList.end(), ServerId, [](const auto & R, uint64_t Id) { return R.ServerId < Id; });
    if (It == AuditAccountServerInfoList.end()) {
        return;
    }
    if (It->ServerId != ServerId) {
        return;
    }
    AuditAccountServerInfoList.erase(It);
}

auto xSL_InternalServerListManager::GetAuditAccountServerInfo(uint64_t ServerId) -> const xSL_AuditAccountServerInfo * {
    auto It = std::lower_bound(AuditAccountServerInfoList.begin(), AuditAccountServerInfoList.end(), ServerId, [](const auto & R, uint64_t Id) { return R.ServerId < Id; });
    if (It == AuditAccountServerInfoList.end()) {
        return nullptr;
    }
    if (It->ServerId != ServerId) {
        return nullptr;
    }
    return &(*It);
}
