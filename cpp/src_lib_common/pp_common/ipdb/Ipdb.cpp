#include "./Ipdb.hpp"

#include "../region.hpp"

#ifdef ZEC_USE_AIWEN_DB
#include <aiwendb.h>
bool xIpDb::Init(const char * DbFilename) {
    auto     MutexGuard = std::lock_guard(DbMutex);
    AWDB_s * AWDBPtr    = new AWDB_s();
    int      status     = AWDB_open(DbFilename, AWDB_MODE_MMAP, AWDBPtr);
    if (AWDB_SUCCESS != status) {
        delete AWDBPtr;
        return false;
    }
    DbPtr            = AWDBPtr;
    this->DbFilename = DbFilename;
    return true;
}

void xIpDb::Reopen() {
    auto MutexGuard = std::lock_guard(DbMutex);
    assert(DbPtr && !DbFilename.empty());
    AWDB_s * AWDBPtr = new AWDB_s();
    int      status  = AWDB_open(DbFilename.c_str(), AWDB_MODE_MMAP, AWDBPtr);
    if (AWDB_SUCCESS != status) {
        delete AWDBPtr;
        return;
    }
    AWDB_close((AWDB_s *)DbPtr);
    delete (AWDB_s *)Steal(DbPtr, AWDBPtr);
    return;
}

void xIpDb::Clean() {
    auto MutexGuard = std::lock_guard(DbMutex);
    if (DbPtr) {
        AWDB_close((AWDB_s *)DbPtr);
        delete (AWDB_s *)Steal(DbPtr);
    }
    DbFilename.clear();
}

xOptional<xGeoIpInfo> xIpDb::Get(const char * IpStr) {
    auto MutexGuard = std::lock_guard(DbMutex);
    if (!DbPtr) {
        return {};
    }

    int                  gai_error, awdb_error;
    AWDB_lookup_result_s result = AWDB_lookup_string((const AWDB_s *)DbPtr, IpStr, &gai_error, &awdb_error);
    if (gai_error || !result.found_entry) {
        return {};
    }
    AWDB_entry_data_s entry_data;
    AWDB_get_value(&result.entry, &entry_data, "adcode", NULL);
    if (entry_data.has_data) {
        std::string CityCodeStr{ entry_data.utf8_string, entry_data.data_size };
        return xGeoIpInfo{ 156, (uint32_t)atoll(CityCodeStr.c_str()) };
    }
    return {};
}

xOptional<xGeoIpInfo> xIpDb::Get(const in_addr_t IpAddr) {
    auto                  MutexGuard = std::lock_guard(DbMutex);
    const unsigned char * AddrPtr    = (const unsigned char *)&IpAddr;
    char                  Buffer[16] = {};
    sprintf(Buffer, "%i.%i.%i.%i", AddrPtr[0], AddrPtr[1], AddrPtr[2], AddrPtr[3]);
    return Get(Buffer);
}

#elif defined(ZEC_USE_IP2LOC_DB)

#include "./IP2Location.h"

bool xIpDb::Init(const char * DbFilename) {
    auto          MutexGuard     = std::lock_guard(DbMutex);
    IP2Location * IP2LocationObj = IP2Location_open(DbFilename);
    if (!IP2LocationObj) {
        return false;
    }
    if (-1 == IP2Location_set_lookup_mode(IP2LocationObj, IP2LOCATION_CACHE_MEMORY)) {
        IP2Location_close(IP2LocationObj);
        return false;
    }
    DbPtr = IP2LocationObj;
    return true;
}
void xIpDb::Reopen() {
}
void xIpDb::Clean() {
    auto          MutexGuard     = std::lock_guard(DbMutex);
    IP2Location * IP2LocationObj = (IP2Location *)Steal(DbPtr);
    IP2Location_close(IP2LocationObj);
}
xOptional<xGeoIpInfo> xIpDb::Get(const char * IpStr) {
    auto MutexGuard = std::lock_guard(DbMutex);
    auto RecordPtr  = IP2Location_get_country_short((IP2Location *)DbPtr, (char *)IpStr);
    auto CleanUp    = xScopeGuard([=] { IP2Location_free_record(RecordPtr); });
    if (RecordPtr && RecordPtr->country_short) {
        X_DEBUG_PRINTF("country: %s", RecordPtr->country_short);
        return xGeoIpInfo{ CountryCodeToCountryId(RecordPtr->country_short) };
    }
    return {};
}
xOptional<xGeoIpInfo> xIpDb::Get(const in_addr_t IpAddr) {
    auto MutexGuard = std::lock_guard(DbMutex);
    Fatal("Not implemented");
    return {};
}

#else
bool xIpDb::Init(const char * DbFilename) {
    return true;
}
void xIpDb::Reopen() {
}
void xIpDb::Clean() {
}
xOptional<xGeoIpInfo> xIpDb::Get(const char * IpStr) {
    return {};
}
xOptional<xGeoIpInfo> xIpDb::Get(const in_addr_t IpAddr) {
    return {};
}

#endif