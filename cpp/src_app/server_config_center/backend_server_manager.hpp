#pragma once
#include <pp_common/base.hpp>
#include <server_arch/service.hpp>

class xCC_BackendServerManager : xService {
public:
    bool Init(xIoContext * ICP, const xNetAddress & BindAddress, const std::string & ForcedBackendServerListFile);
    void Clean();
    void Tick(uint64_t NowMS);

    auto & GetServerList() const {
        return ServerList;
    }

protected:
    void ReloadServerList();
    bool OnClientPacket(xServiceClientConnection & Connection, const xPacketHeader & Header, ubyte * PayloadPtr, size_t PayloadSize) override;

    bool OnDownloadBackendServerList(xServiceClientConnection & Connection, const xPacketHeader & Header);

public:
    uint64_t                 LastReloadTimestamp;
    std::string              ServerListFile;
    std::vector<xNetAddress> ServerList;
};
