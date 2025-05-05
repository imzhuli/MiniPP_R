#pragma once
#include "../base.hpp"

struct xPPI_DeviceInfoUpdate : public xBinaryMessage {  // from proxy_access to relay server
public:
    void SerializeMembers() override {
        W(DeviceUuid, Version, RelayServerUuid, PrimaryIpv4Address, PrimaryIpv6Address);
        W(IsOffline, SupportUdpChannel, SupportDnsRequests, SpeedLimitEnabled);
    }
    void DeserializeMembers() override {
    }

    std::string DeviceUuid;
    uint32_t    Version;
    std::string RelayServerUuid;
    xNetAddress PrimaryIpv4Address;
    xNetAddress PrimaryIpv6Address;

    xContinentId ContinentId;
    xCountryId   CountryId;
    xStateId     StateId;
    xCityId      CityId;

    bool IsOffline;  // 下线时会设置这个标志位. 但考虑到服务器的变更, 不是所有设备都有上/下线标志, 后台服务器应当将30分钟未发统计的设备设为下线.
    bool SupportUdpChannel;
    bool SupportDnsRequests;
    bool SpeedLimitEnabled;

    //
};
