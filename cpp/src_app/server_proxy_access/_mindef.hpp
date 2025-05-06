#pragma once
#include <pp_common/base.hpp>

static constexpr const size_t   REQUEST_POOL_SIZE               = 5'0000;
static constexpr const size_t   DEFAULT_AUTH_CACHE_POOL_SIZE    = 10'0000;
static constexpr const uint64_t AUTH_CACHE_REQUEST_TIMEOUT_MS   = 5'000;
static constexpr const uint64_t AUTH_CACHE_RECONFIRM_TIMEOUT_MS = 1 * 60 * 1000;
static constexpr const uint64_t AUTH_CACHE_NODE_TIMEOUT_MS      = 2 * 60'000;

struct xPA_AccountCheckNode : xListNode {
    uint64_t CheckAccountTimestampMS = 0;
};

struct xPA_DeviceRequest {
    uint64_t   ClientConnectionId;
    xCountryId CountryId;
    xStateId   StateId;
    xCityId    CityId;
    bool       RequireIpv6;
    bool       RequireUdp;
    bool       RequireRemoteDns;
};

struct xPA_DeviceRequestResp {
    uint64_t ClientConnectionId;
    uint64_t RelayServerRuntimeId;
    uint64_t DeviceRelaySideId;
};

struct xPA_AuthResult {

    xCountryId CountryId;
    xStateId   StateId;
    xCityId    CityId;
    bool       IsBlocked;
    bool       RequireIpv6;
    bool       RequireUdp;
    bool       RequireRemoteDns;
    bool       AutoChangeIp;

    bool Ready;
};
