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
    static constexpr const size_t MaxAuthCacheServerCount            = xPP_DownloadAuthCacheServerList::MAX_SERVER_SIZE;
    static constexpr const size_t MaxDeviceAuditCollectorServerCount = xPP_DownloadDeviceAuditCollectorServerList::MAX_SERVER_SIZE;

    auto AddAuthCacheServerInfo(uint64_t ServerId, xNetAddress ServerAddress) -> xSL_AuthCacheServerInfo *;
    void RemoveAuthCacheServerInfo(uint64_t ServerId);
    auto GetAuthCacheServerInfo(uint64_t ServerId) -> const xSL_AuthCacheServerInfo *;
    auto GetAuthCacheServerInfoList() const -> const std::vector<std::unique_ptr<xSL_AuthCacheServerInfo>> & { return AuthCacheServerInfoList; }

private:
    uint32_t                                              AuthCacheServerInfoListVersion          = 0;
    bool                                                  AuthCacheServerInfoListDirty            = false;
    uint64_t                                              AuthCacheServerInfoListVersionTimestamp = 0;
    std::vector<std::unique_ptr<xSL_AuthCacheServerInfo>> AuthCacheServerInfoList;

    uint32_t                                                DeviceAuditServerInfoListVersion          = 0;
    bool                                                    DeviceAuditServerInfoListDirty            = false;
    uint64_t                                                DeviceAuditServerInfoListVersionTimestamp = 0;
    std::vector<std::unique_ptr<xSL_DeviceAuditServerInfo>> DeviceAuditServerInfoList;

    uint32_t                                                          AccountAuditCollectorServerInfoListVersion          = 0;
    bool                                                              AccountAuditCollectorServerInfoListDirty            = false;
    uint64_t                                                          AccountAuditCollectorServerInfoListVersionTimestamp = 0;
    std::vector<std::unique_ptr<xSL_AccountAuditCollectorServerInfo>> AccountAuditCollectorServerInfoList;
};
