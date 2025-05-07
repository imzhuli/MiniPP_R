#pragma once
#include <pp_common/base.hpp>

struct xPA_LocalAudit {

    uint64_t BlockedAuthNodeCount = 0;

    uint64_t AuthRequestCountPerMinute       = 0;
    uint64_t ActureAuthRequestCountPerMinute = 0;
    uint64_t DeviceSelectionPerMinute        = 0;

    uint64_t AuthCacheNodeCount       = 0;
    uint64_t OngoingAuthRequestNumber = 0;

    uint64_t Error_MissingRelayServerInfo    = 0;
    uint64_t Error_LostRelayServerConnection = 0;

public:
    std::string ToString() const;
};
