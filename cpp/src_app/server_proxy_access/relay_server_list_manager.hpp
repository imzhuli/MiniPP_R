#pragma once
#include <compare>
#include <pp_common/base.hpp>
#include <server_arch/client_pool.hpp>

struct xPA_RelayServerInfo {
    uint32_t    RuntimeServerId;
    xNetAddress AddressForPA;
};

inline std::strong_ordering operator<=>(const xPA_RelayServerInfo & lhs, const xPA_RelayServerInfo & rhs) {
    return lhs.RuntimeServerId <=> rhs.RuntimeServerId;
}

class xPA_RelayServerListManager : public xClientPool {

public:
    bool Init(xIoContext * ICP, const std::vector<xNetAddress> & AddressList);
    void Clean();
    void Tick(uint64_t NowMS);

    void StartDownloadRelayServerList();

protected:
    void OnServerConnected(xClientConnection & CC) override;
    void OnServerClose(xClientConnection & CC) override;
    bool OnServerPacket(xClientConnection & CC, xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) override;

private:
    std::vector<uint64_t>            ConfigCenterConnectionIdList;
    std::vector<xPA_RelayServerInfo> RelayServerIdList;

    size_t   AvailableServerCount         = 0;
    uint64_t DownloadRoundCounter         = 0;
    uint64_t LastDownloadStartTimestampMS = 0;
};
