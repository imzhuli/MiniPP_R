#pragma once
#include <pp_common/base.hpp>

class xDnsQuery : public xBinaryMessage {
public:
    void SerializeMembers() override {
        W(Hostname);
    }
    void DeserializeMembers() override {
        R(Hostname);
    }

public:
    std::string Hostname;
};

class xDnsQueryResp : public xBinaryMessage {
public:
    void SerializeMembers() override {
        W(Hostname, PrimaryIpv4, PrimaryIpv6);
    }
    void DeserializeMembers() override {
        R(Hostname, PrimaryIpv4, PrimaryIpv6);
    }

public:
    std::string Hostname;
    xNetAddress PrimaryIpv4;
    xNetAddress PrimaryIpv6;
};
