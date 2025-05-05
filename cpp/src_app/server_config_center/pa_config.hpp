#pragma once
#include <pp_common/base.hpp>
#include <pp_protocol/cc_pa/relay_info.hpp>
#include <server_arch/service.hpp>

class xCC_PAConfigManager : public xService {
public:
    bool Init(xIoContext * ICP, const xNetAddress & BindAddress);
    void Clean();

    bool OnClientPacket(xServiceClientConnection & Connection, xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize)
        override;

public:
    bool OnQueryRelayServerListVersion(xServiceClientConnection & Connection, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize);
    bool OnDownloadRelayServerList(xServiceClientConnection & Connection, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize);
    bool BuildTotalList();

private:
    uint32_t                            Version = 0;
    std::vector<xCC_PA_RelayServerInfo> LocalTotalRelayServerList;
};
