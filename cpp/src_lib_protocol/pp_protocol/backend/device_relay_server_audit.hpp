#pragma once
#include <pp_common/base.hpp>

class xDeviceInfoAudit : public xBinaryMessage {
public:
    void SerializeMembers() override {
        W(ServerUuid);
        W(RuntimeServerId);
        W(Ipv4CtrlExportAddress);
        W(Ipv4DataExportAddress);
        W(Ipv4ProxyExportAddress);
        W(AvailableDeviceCount);
        W(ActiveProxyConnectionCount);
        W(ActiveRelayChannelCount);
        W(AverageDeviceLifeDurationInSecInLast24Hours);
    }
    void DeserializeMembers() override {
        R(ServerUuid);
        R(RuntimeServerId);
        R(Ipv4CtrlExportAddress);
        R(Ipv4DataExportAddress);
        R(Ipv4ProxyExportAddress);
        R(AvailableDeviceCount);
        R(ActiveProxyConnectionCount);
        R(ActiveRelayChannelCount);
        R(AverageDeviceLifeDurationInSecInLast24Hours);
    }

public:
    std::string ServerUuid;
    uint64_t    RuntimeServerId;
    xNetAddress Ipv4CtrlExportAddress;
    xNetAddress Ipv4DataExportAddress;
    xNetAddress Ipv4ProxyExportAddress;
    uint32_t    AvailableDeviceCount;
    uint32_t    ActiveProxyConnectionCount;
    uint32_t    ActiveRelayChannelCount;
    uint32_t    AverageDeviceLifeDurationInSecInLast24Hours;
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
