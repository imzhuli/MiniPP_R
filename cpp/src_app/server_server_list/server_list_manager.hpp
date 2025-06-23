#pragma once
#include <pp_common/base.hpp>
#include <pp_protocol/internal/all.hpp>

struct xSL_InternalServerBase : xListNode {
    eServerType ServerType;
    uint64_t    ServerId;
    xNetAddress ServerAddress;
};

using xSL_AuthCacheServerInfo             = xSL_InternalServerBase;
using xSL_DeviceAuditServerInfo           = xSL_InternalServerBase;
using xSL_AccountAuditCollectorServerInfo = xSL_InternalServerBase;

class xSL_InternalServerListManager : public xService {
public:
    bool Init(xIoContext * IoContextPtr, const xNetAddress & BindAddress, size_t MaxConnectionId) {
        AuthCacheServerInfoListVersionTimestampMS             = GetTickTimeMS();
        DeviceAuditServerInfoListVersionTimestampMS           = GetTickTimeMS();
        AccountAuditCollectorServerInfoListVersionTimestampMS = GetTickTimeMS();
        return xService::Init(IoContextPtr, BindAddress, MaxConnectionId, true);
    };
    void Clean() {
        Reset(AuthCacheServerInfoListVersion);
        Reset(AuthCacheServerInfoListDirty);
        Reset(AuthCacheServerInfoListVersionTimestampMS);
        Reset(AuthCacheServerInfoList);
        Reset(VersionedAuthCacheServerInfoList);

        Reset(DeviceAuditServerInfoListVersion);
        Reset(DeviceAuditServerInfoListDirty);
        Reset(DeviceAuditServerInfoListVersionTimestampMS);
        Reset(DeviceAuditServerInfoList);
        Reset(VersionedDeviceAuditServerInfoList);

        Reset(AccountAuditCollectorServerInfoListVersion);
        Reset(AccountAuditCollectorServerInfoListDirty);
        Reset(AccountAuditCollectorServerInfoListVersionTimestampMS);
        Reset(AccountAuditCollectorServerInfoList);
        Reset(VersionedAccountAuditCollectorServerInfoList);
        xService::Clean();
    }

    void OnTick(uint64_t NowMS) override;

    auto AddAuthCacheServerInfo(uint64_t ServerId, xNetAddress ServerAddress) -> xSL_AuthCacheServerInfo *;
    void RemoveAuthCacheServerInfo(uint64_t ServerId);
    auto GetAuthCacheServerInfo(uint64_t ServerId) -> const xSL_AuthCacheServerInfo *;
    auto GetAuthCacheServerInfoList() const -> const std::vector<xSL_AuthCacheServerInfo> & { return VersionedAuthCacheServerInfoList; }
    auto GetAuthCacheServerInfoListVersion() const -> uint32_t { return AuthCacheServerInfoListVersion; }

private:
    xTicker Ticker;

    uint32_t                             AuthCacheServerInfoListVersion            = 0;
    bool                                 AuthCacheServerInfoListDirty              = false;
    uint64_t                             AuthCacheServerInfoListVersionTimestampMS = {};
    std::vector<xSL_AuthCacheServerInfo> AuthCacheServerInfoList;
    std::vector<xSL_AuthCacheServerInfo> VersionedAuthCacheServerInfoList;

    uint32_t                               DeviceAuditServerInfoListVersion            = 0;
    bool                                   DeviceAuditServerInfoListDirty              = false;
    uint64_t                               DeviceAuditServerInfoListVersionTimestampMS = {};
    std::vector<xSL_DeviceAuditServerInfo> DeviceAuditServerInfoList;
    std::vector<xSL_DeviceAuditServerInfo> VersionedDeviceAuditServerInfoList;

    uint32_t                                         AccountAuditCollectorServerInfoListVersion            = 0;
    bool                                             AccountAuditCollectorServerInfoListDirty              = false;
    uint64_t                                         AccountAuditCollectorServerInfoListVersionTimestampMS = {};
    std::vector<xSL_AccountAuditCollectorServerInfo> AccountAuditCollectorServerInfoList;
    std::vector<xSL_AccountAuditCollectorServerInfo> VersionedAccountAuditCollectorServerInfoList;
};
