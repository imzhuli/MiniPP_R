#pragma once
#include "./base.hpp"

static constexpr const xContinentId WildContinentId = xContinentId(-1);

struct xRegionId {
    xCountryId CountryId;
    xStateId   StateId;
    xCityId    CityId;
};

struct xRegionInfo {
    xRegionId RegionId;

    std::string CountryName;
    std::string CityName;
    std::string ShortCityName;
};

extern uint32_t CountryCodeToCountryId(const char * CC);
