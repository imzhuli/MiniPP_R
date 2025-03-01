#include "./server_list.hpp"

#include <sstream>

bool xServerInfoListDownloader::Init() {
    return true;
}

void xServerInfoListDownloader::Clean() {
    Renew(ServerList);
}

void xServerInfoListDownloader::PostDownloadRequest(xTcpConnection * Conn) {
}

bool xServerInfoListDownloader::OnDownloadResponse(void * Payload, size_t PayloadSize) {

    return false;
}

//////////////

bool xServerInfoManager::Init() {
    TotalUsedSlots = 0;
    RuntimeAssert(ServerIdManager.Init());
    return true;
}

void xServerInfoManager::Clean() {
    for (auto & S : ServerInfoList) {
        Renew(S);
    }
    ServerIdManager.Clean();
    RenewValue(TotalUsedSlots);
}

uint64_t xServerInfoManager::AddServerInfo(const xNetAddress & ServerAddress, const std::string & ServerInfoEx) {
    if (TotalUsedSlots >= ServerInfoList.size()) {
        return 0;
    }
    auto Id = ServerIdManager.Acquire();
    assert(Id);
    uint32_t Index   = Id - 1;
    uint32_t Stamp   = (uint32_t)GetTimestampUS();
    auto &   SI      = ServerInfoList[Index];
    SI.ServerKey     = Make64(Index, Stamp);
    SI.ServerAddress = ServerAddress;
    SI.ServerInfoEx  = ServerInfoEx;
    ++TotalUsedSlots;
    ++Version;
    return SI.ServerKey;
}

void xServerInfoManager::RemoveServerByKey(uint64_t Key) {
    auto Index = High32(Key);
    return RemoveServerByIndex(Index);
}

void xServerInfoManager::RemoveServerByIndex(uint32_t Index) {
    assert(Index < ServerInfoList.size());
    auto & SI = ServerInfoList[Index];
    assert(SI.IsInUse());

    Renew(SI);
    auto ReleaseId = Index + 1;
    ServerIdManager.Release(ReleaseId);
    --TotalUsedSlots;
    ++Version;
}

std::string xServerInfoManager::Dump() const {
    auto S = std::ostringstream();
    S << "Version: " << Version << ", Total: " << TotalUsedSlots << endl;

    for (size_t I = 0; I < ServerInfoList.size(); ++I) {
        auto & SI = ServerInfoList[I];
        if (!SI.IsInUse()) {
            continue;
        }
        S << "{ " << I << ": Key: " << SI.ServerKey << ", Address: " << SI.ServerAddress.ToString() << ", Ex: " << SI.ServerInfoEx << "}" << endl;
    }
    return S.str();
}
