#pragma once
#include "../base.hpp"

#include <netinet/in.h>

#include <mutex>

#if !defined(ZEC_USE_AIWEN_DB) && !defined(ZEC_USE_IP2LOC_DB)
#define ZEC_USE_IP2LOC_DB
#endif

struct xGeoIpInfo {
    uint32_t CountryCode = 0;
    uint32_t CityCode    = 0;
};

class xIpDb : xNonCopyable {
public:
    bool                  Init(const char * DbFilename);
    void                  Reopen();
    void                  Clean();
    xOptional<xGeoIpInfo> Get(const char * IpStr);
    xOptional<xGeoIpInfo> Get(const in_addr_t IpAddr);

private:
    void *      DbPtr = nullptr;
    std::string DbFilename;
    std::mutex  DbMutex;
};
