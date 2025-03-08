#pragma once
#include <pp_common/base.hpp>

class xDeviceInfoAudit : public xBinaryMessage {
public:
    void SerializeMembers() override {
        W(Version, DeviceId, RelayServerId, Ipv4Address, Ipv6Address);
        W(DnsEnabled, Ipv6Enabled, Ipv6Preferred, SpeedLimitEnabled);
        W(TotalOnlineTimestampMS, TotalUpstreamSize, TotalDownstreamSize, TotalConnectionEnabled, TotalUdpChannelEnabled);
        W(CurrentConnectionCount, CurrentUdpChannelCount);
    }
    void DeserializeMembers() override {
        R(Version, DeviceId, RelayServerId, Ipv4Address, Ipv6Address);
        R(DnsEnabled, Ipv6Enabled, Ipv6Preferred, SpeedLimitEnabled);
        R(TotalOnlineTimestampMS, TotalUpstreamSize, TotalDownstreamSize, TotalConnectionEnabled, TotalUdpChannelEnabled);
        R(CurrentConnectionCount, CurrentUdpChannelCount);
    }

public:
    uint32_t    Version;
    std::string DeviceId;
    uint64_t    RelayServerId;  // device id from device_relay_server, if this is zero, device is offline
    xNetAddress Ipv4Address;    // never unavailabe, but may be different from real address
    xNetAddress Ipv6Address;    // note: might be unavailable address

    bool DnsEnabled;
    bool Ipv6Enabled;  // may be disabled event Ipv6Address is available
    bool Ipv6Preferred;
    bool SpeedLimitEnabled;

    uint64_t TotalOnlineTimestampMS;
    uint64_t TotalUpstreamSize;
    uint64_t TotalDownstreamSize;
    uint32_t TotalConnectionEnabled;
    uint32_t TotalUdpChannelEnabled;

    uint32_t CurrentConnectionCount;
    uint32_t CurrentUdpChannelCount;
};

class xDeviceInfoAuditResp : public xBinaryMessage {
public:
    void SerializeMembers() override {
        Pure();
    }
    void DeserializeMembers() override {
        Pure();
    }
};
