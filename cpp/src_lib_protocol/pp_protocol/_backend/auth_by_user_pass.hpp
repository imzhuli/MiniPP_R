#pragma once
#include "../base.hpp"

class xPPB_AuthByUserPass : public xBinaryMessage {
public:
    void SerializeMembers() override {
        W(UserPass);
        // W(ClientIp);
    }
    void DeserializeMembers() override {
        R(UserPass);
        R(ClientIp);
    }

public:
    std::string UserPass;
    xNetAddress ClientIp;
};

class xPPB_AuthByUserPassResp : public xBinaryMessage {
public:
    void SerializeMembers() override {
        W(ErrorCode);
        if (!ErrorCode) {
            W(AuditId, ContinentId, CountryId, StateId, CityId, Duration, Random);
            W(AutoChangeIpOnDeviceOffline, EnableUdp, Ipv6Prefered);
            W(Redirect, AuditIdForThirdPartyResource);
        }
        if (xBinaryMessageWriter::HasError()) {
            X_DEBUG_PRINTF("HasError");
        }
    }
    void DeserializeMembers() override {
        R(ErrorCode);
        if (!ErrorCode) {
            R(AuditId, ContinentId, CountryId, StateId, CityId, Duration, Random);
            R(AutoChangeIpOnDeviceOffline, EnableUdp, Ipv6Prefered);
            R(Redirect, AuditIdForThirdPartyResource);
        }
        if (xBinaryMessageReader::HasError()) {
            X_DEBUG_PRINTF("HasError");
        }
    }

public:
    uint32_t     ErrorCode;
    xAuditId     AuditId;
    xContinentId ContinentId;
    xCountryId   CountryId;
    xStateId     StateId;
    xCityId      CityId;
    uint32_t     Duration;  // min, 0 for change ip every time
    uint32_t     Random;
    bool         AutoChangeIpOnDeviceOffline;
    bool         EnableUdp;
    bool         Ipv6Prefered;
    std::string  PoolFlags;
    std::string  Redirect;
    uint32_t     AuditIdForThirdPartyResource;
};
