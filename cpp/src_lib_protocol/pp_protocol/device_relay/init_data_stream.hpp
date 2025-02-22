#pragma once
#include <pp_common/base.hpp>

class xInitDataStream : public xBinaryMessage {
public:
    void SerializeMembers() override {
        W(CtrlId, DataKey, DeviceLocalIdString);
    }
    void DeserializeMembers() override {
        R(CtrlId, DataKey, DeviceLocalIdString);
    }

public:
    uint64_t    CtrlId;
    std::string DataKey;
    std::string DeviceLocalIdString;
};

class xInitDataStreamResp : public xBinaryMessage {
public:
    void SerializeMembers() override {
        W(Accepted);
    }
    void DeserializeMembers() override {
        R(Accepted);
    }

public:
    bool Accepted;
};
