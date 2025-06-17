#pragma once
#include <pp_common/base.hpp>
#include <server_arch/service.hpp>

class xSL_BackendServerManager : xService {
public:
    bool Init(xIoContext * ICP, const xNetAddress & BindAddress, const std::string & ForcedBackendServerListFile);
    void Clean();
    void Tick(uint64_t NowMS);

    auto & GetServerList() const { return ServerList; }

protected:
    void ReloadServerList();
    bool OnClientPacket(xServiceClientConnection & Connection, xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) override;

    bool OnDownloadBackendServerList(xServiceClientConnection & Connection, xPacketCommandId CommandId, xPacketRequestId RequestId);

public:
    uint64_t                 LastReloadTimestamp;
    std::string              ServerListFile;
    std::vector<xNetAddress> ServerList;
};
