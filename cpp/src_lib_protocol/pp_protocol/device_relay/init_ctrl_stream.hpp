#pragma once
#include <pp_common/base.hpp>

class xInitCtrlStream : public xBinaryMessage {
public:
    void SerializeMembers() override {
        W(CheckKey, Ipv4Address, Ipv6Address);
    }
    void DeserializeMembers() override {
        R(CheckKey, Ipv4Address, Ipv6Address);
    }

public:
    std::string CheckKey;
    xNetAddress Ipv4Address;
    xNetAddress Ipv6Address;
};

class xInitCtrlStreamResp : public xBinaryMessage {
public:
    void SerializeMembers() override {
        W(DeviceId, CtrlId, DeviceKey);
    }
    void DeserializeMembers() override {
        R(DeviceId, CtrlId, DeviceKey);
    }

public:
    uint64_t    DeviceId;
    uint64_t    CtrlId;
    std::string DeviceKey;
};
