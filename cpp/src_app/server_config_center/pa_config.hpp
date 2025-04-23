#pragma once
#include <pp_common/base.hpp>
#include <server_arch/service.hpp>
#include <pp_protocol/cc_pa/relay_info.hpp>

class xCC_PAConfigManager : public xService {
public:
    bool Init(xIoContext * ICP, const xNetAddress & BindAddress);
    void Clean();

    bool OnPacket(xServiceClientConnection & Connection, const xPacketHeader & Header, ubyte * PayloadPtr, size_t PayloadSize) override;

public:
    bool OnQueryRelayServerListVersion(xServiceClientConnection & Connection, const xPacketHeader & Header, ubyte * PayloadPtr, size_t PayloadSize);
    bool OnDownloadRelayServerList(xServiceClientConnection & Connection, const xPacketHeader & Header, ubyte * PayloadPtr, size_t PayloadSize);
    bool BuildTotalList();

private:
    uint32_t                            Version = 0;
    std::vector<xCC_PA_RelayServerInfo> LocalTotalRelayServerList;
};
