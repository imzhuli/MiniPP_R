#include "ip_location.hpp"

static constexpr const int64_t IpDbReloadInterval = 60'000;

bool xCC_IpLocationManager::Init(const std::string & DbName) {
    IpDbName = DbName;
    ReloadIpDB();
    return true;
}

void xCC_IpLocationManager::Clean() {
    // TODO: cleanup ipdb
}

void xCC_IpLocationManager::Tick(uint64_t NowMS) {
    auto Interval = SignedDiff(NowMS, LastUpdateDbTime);
    if (Interval < IpDbReloadInterval) {
        return;
    }
    LastUpdateDbTime = NowMS;
    ReloadIpDB();
}

void xCC_IpLocationManager::ReloadIpDB() {
    assert(IpDbName.size());

    //
}

xRegionId xCC_IpLocationManager::GetRegionByIp(const char * IpString) {
    return xRegionId();
}

xContinentId xCC_IpLocationManager::GetContinentIdByCountry(xCountryId CountryId) {
    return 0;
}
