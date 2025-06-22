#pragma once
#include <pp_common/base.hpp>

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
    bool Init();
    void Clean();

    bool AddAuthCacheServerInfo(uint64_t ServerId, xNetAddress ServerAddress);
    void RemoveAuthCacheServerInfo(uint64_t ServerId);

private:
    std::vector<std::unique_ptr<xSL_AuthCacheServerInfo>>             AuthCacheServerInfoList;
    std::vector<std::unique_ptr<xSL_DeviceAuditServerInfo>>           DeviceAuditServerInfoList;
    std::vector<std::unique_ptr<xSL_AccountAuditCollectorServerInfo>> AccountAuditCollectorServerInfoList;
};
