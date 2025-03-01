#pragma once
#include <array>
#include <object/object.hpp>
#include <pp_common/base.hpp>

struct xServerInfo {
    uint64_t    ServerKey = 0;
    xNetAddress ServerAddress;
    std::string ServerInfoEx;

    uint32_t GetServerIndex() const {
        return High32(ServerKey);
    }
    uint32_t IsInUse() const {
        return Low32(ServerKey);
    }
};

class xServerInfoListDownloader {
public:
    bool Init();
    void Clean();

    void PostDownloadRequest(xTcpConnection * Conn);
    bool OnDownloadResponse(void * Payload, size_t PayloadSize);

protected:
    uint32_t                 Version;
    std::vector<xServerInfo> ServerList;
    //
};

class xServerInfoManager {
public:
    bool Init();
    void Clean();

    uint64_t AddServerInfo(const xNetAddress & ServerAddress, const std::string & ServerInfoEx = {});
    void     RemoveServerByKey(uint64_t Key);
    void     RemoveServerByIndex(uint32_t Index);

    std::string Dump() const;

private:
    uint32_t                      Version        = 0;
    uint32_t                      TotalUsedSlots = 0;
    xel::xObjectIdManagerMini     ServerIdManager;
    std::array<xServerInfo, 4096> ServerInfoList;
};
