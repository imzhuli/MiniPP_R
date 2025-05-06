#pragma once
#include "../base.hpp"

struct xQueryAuthCache : xBinaryMessage {

    void SerializeMembers() override {
        W(UserPass);
    }
    void DeserializeMembers() override {
        R(UserPass);
    }

    std::string_view UserPass;
    //
};

struct xQueryAuthCacheResp : xBinaryMessage {

    void SerializeMembers() override {
        W(CountryId);
        W(StateId);
        W(CityId);
        W(IsBlocked);
        W(RequireIpv6);
        W(RequireUdp);
        W(RequireRemoteDns);
        W(AutoChangeIp);
    }
    void DeserializeMembers() override {
        R(CountryId);
        R(StateId);
        R(CityId);
        R(IsBlocked);
        R(RequireIpv6);
        R(RequireUdp);
        R(RequireRemoteDns);
        R(AutoChangeIp);
    }

    xCountryId CountryId;
    xStateId   StateId;
    xCityId    CityId;
    bool       IsBlocked;
    bool       RequireIpv6;
    bool       RequireUdp;
    bool       RequireRemoteDns;
    bool       AutoChangeIp;
    //
};
