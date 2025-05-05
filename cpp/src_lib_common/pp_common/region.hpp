#pragma once
#include "./base.hpp"

static constexpr const xContinentId WildContinentId = xContinentId(-1);

struct xGeoInfo {
    xCountryId CountryId;
    xStateId   StateId;
    xCityId    CityId;
};

struct xGeoInfoEx {
    xGeoInfo RegionId;

    std::string CountryName;
    std::string CityName;
    std::string ShortCityName;
};

extern uint32_t CountryCodeToCountryId(const char * CC);
