#pragma once
#include "../base.hpp"

struct xQueryAuthCache : xBinaryMessage {

    void SerializeMembers() override { W(UserPass); }
    void DeserializeMembers() override { R(UserPass); }

    std::string_view UserPass;
    //
};

struct xQueryAuthCacheResp : xBinaryMessage {

    void SerializeMembers() override {
        W(AuditId);
        W(CountryId);
        W(StateId);
        W(CityId);
        W(IsBlocked);
        W(RequireIpv6);
        W(RequireUdp);
        W(RequireRemoteDns);
        W(AutoChangeIp);
        W(PAToken);
    }
    void DeserializeMembers() override {
        R(AuditId);
        R(CountryId);
        R(StateId);
        R(CityId);
        R(IsBlocked);
        R(RequireIpv6);
        R(RequireUdp);
        R(RequireRemoteDns);
        R(AutoChangeIp);
        R(PAToken);
    }

    xAuditId         AuditId;
    xCountryId       CountryId;
    xStateId         StateId;
    xCityId          CityId;
    bool             IsBlocked;
    bool             RequireIpv6;
    bool             RequireUdp;
    bool             RequireRemoteDns;
    bool             AutoChangeIp;
    std::string_view PAToken;
    //
};
