#pragma once
#include <pp_common/base.hpp>
#include <pp_protocol/internal/all.hpp>

struct xSL_InternalServerBase : xListNode {
    eServerType ServerType;
    uint64_t    ServerId;
    xNetAddress ServerAddress;
};

using xSL_AuthCacheServerInfo    = xSL_InternalServerBase;
using xSL_AuditDeviceServerInfo  = xSL_InternalServerBase;
using xSL_AuditAccountServerInfo = xSL_InternalServerBase;

class xSL_InternalServerListManager {
public:
    bool Init() {
        AuthCacheServerInfoListVersionTimestampMS    = Ticker();
        AuditDeviceServerInfoListVersionTimestampMS  = Ticker();
        AuditAccountServerInfoListVersionTimestampMS = Ticker();
        return true;
    };
    void Clean() {
        Reset(AuthCacheServerInfoListVersion);
        Reset(AuthCacheServerInfoListDirty);
        Reset(AuthCacheServerInfoListVersionTimestampMS);
        Reset(AuthCacheServerInfoList);
        Reset(VersionedAuthCacheServerInfoList);

        Reset(AuditDeviceServerInfoListVersion);
        Reset(AuditDeviceServerInfoListDirty);
        Reset(AuditDeviceServerInfoListVersionTimestampMS);
        Reset(AuditDeviceServerInfoList);
        Reset(VersionedAuditDeviceServerInfoList);

        Reset(AuditAccountServerInfoListVersion);
        Reset(AuditAccountServerInfoListDirty);
        Reset(AuditAccountServerInfoListVersionTimestampMS);
        Reset(AuditAccountServerInfoList);
        Reset(VersionedAuditAccountServerInfoList);
    }

    void OnTick(uint64_t NowMS);

    auto AddAuthCacheServerInfo(uint64_t ServerId, xNetAddress ServerAddress) -> xSL_AuthCacheServerInfo *;
    void RemoveAuthCacheServerInfo(uint64_t ServerId);
    auto GetAuthCacheServerInfo(uint64_t ServerId) -> const xSL_AuthCacheServerInfo *;
    auto GetAuthCacheServerInfoList() const -> const std::vector<xSL_AuthCacheServerInfo> & { return VersionedAuthCacheServerInfoList; }
    auto GetAuthCacheServerInfoListVersion() const -> uint32_t { return AuthCacheServerInfoListVersion; }

    auto AddAuditDeviceServerInfo(uint64_t ServerId, xNetAddress ServerAddress) -> xSL_AuditDeviceServerInfo *;
    void RemoveAuditDeviceServerInfo(uint64_t ServerId);
    auto GetAuditDeviceServerInfo(uint64_t ServerId) -> const xSL_AuditDeviceServerInfo *;
    auto GetAuditDeviceServerInfoList() const -> const std::vector<xSL_AuditDeviceServerInfo> & { return VersionedAuditDeviceServerInfoList; }
    auto GetAuditDeviceServerInfoListVersion() const -> uint32_t { return AuditDeviceServerInfoListVersion; }

    auto AddAuditAccountServerInfo(uint64_t ServerId, xNetAddress ServerAddress) -> xSL_AuditAccountServerInfo *;
    void RemoveAuditAccountServerInfo(uint64_t ServerId);
    auto GetAuditAccountServerInfo(uint64_t ServerId) -> const xSL_AuditAccountServerInfo *;
    auto GetAuditAccountServerInfoList() const -> const std::vector<xSL_AuditAccountServerInfo> & { return VersionedAuditAccountServerInfoList; }
    auto GetAuditAccountServerInfoListVersion() const -> uint32_t { return AuditAccountServerInfoListVersion; }

private:
    xTicker Ticker;

    uint32_t                             AuthCacheServerInfoListVersion            = 0;
    bool                                 AuthCacheServerInfoListDirty              = false;
    uint64_t                             AuthCacheServerInfoListVersionTimestampMS = {};
    std::vector<xSL_AuthCacheServerInfo> AuthCacheServerInfoList;
    std::vector<xSL_AuthCacheServerInfo> VersionedAuthCacheServerInfoList;

    uint32_t                               AuditDeviceServerInfoListVersion            = 0;
    bool                                   AuditDeviceServerInfoListDirty              = false;
    uint64_t                               AuditDeviceServerInfoListVersionTimestampMS = {};
    std::vector<xSL_AuditDeviceServerInfo> AuditDeviceServerInfoList;
    std::vector<xSL_AuditDeviceServerInfo> VersionedAuditDeviceServerInfoList;

    uint32_t                                AuditAccountServerInfoListVersion            = 0;
    bool                                    AuditAccountServerInfoListDirty              = false;
    uint64_t                                AuditAccountServerInfoListVersionTimestampMS = {};
    std::vector<xSL_AuditAccountServerInfo> AuditAccountServerInfoList;
    std::vector<xSL_AuditAccountServerInfo> VersionedAuditAccountServerInfoList;
};
