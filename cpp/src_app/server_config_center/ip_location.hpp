#pragma once
#include <pp_common/base.hpp>
#include <pp_common/region.hpp>

class xCC_IpLocationManager {
public:
    bool Init(const std::string & DbName);
    void Clean();
    void Tick(uint64_t NowMS);
    void ReloadIpDB();

    xRegionId    GetRegionByIp(const char * IpString);
    xContinentId GetContinentIdByCountry(xCountryId CountryId);

private:
    std::string IpDbName;
    uint64_t    LastUpdateDbTime = 0;
};
