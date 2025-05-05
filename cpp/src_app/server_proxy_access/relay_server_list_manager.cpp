#include "./relay_server_list_manager.hpp"

#include <pp_protocol/cc_pa/relay_info.hpp>
#include <pp_protocol/command.hpp>

bool xPA_RelayServerListManager::Init(xIoContext * ICP, const std::vector<xNetAddress> & AddressList) {
    RuntimeAssert(!AddressList.empty() && AddressList.size() < 1024);
    RuntimeAssert(xClientPool::Init(ICP, 1024));
    for (auto & A : AddressList) {
        auto CID = xClientPool::AddServer(A);
        RuntimeAssert(CID);
        ConfigCenterConnectionIdList.push_back(CID);
    }
    for (auto & CID : ConfigCenterConnectionIdList) {
        X_DEBUG_PRINTF("ConnectionId to ConfigCenter: %" PRIx64 "", CID);
    }

    return true;
}

void xPA_RelayServerListManager::Clean() {
    Reset(AvailableServerCount);
    Reset(DownloadRoundCounter);
    Reset(ConfigCenterConnectionIdList);
    xClientPool::Clean();
}

void xPA_RelayServerListManager::Tick(uint64_t NowMS) {
    xClientPool::Tick(NowMS);
    auto Restart = NowMS - LastDownloadStartTimestampMS >= 10 * 60'000;
    if (Restart && AvailableServerCount) {
        LastDownloadStartTimestampMS = NowMS;
        StartDownloadRelayServerList();
    }
}

void xPA_RelayServerListManager::OnServerConnected(xClientConnection & CC) {
    X_DEBUG_PRINTF("ConnectionId=%" PRIx64 ", TargetAddress=%s", CC.GetConnectionId(), CC.GetTargetAddress().ToString().c_str());
    ++AvailableServerCount;
}

void xPA_RelayServerListManager::OnServerClose(xClientConnection & CC) {
    X_DEBUG_PRINTF("ConnectionId=%" PRIx64 ", TargetAddress=%s", CC.GetConnectionId(), CC.GetTargetAddress().ToString().c_str());
    --AvailableServerCount;
}

bool xPA_RelayServerListManager::OnServerPacket(
    xClientConnection & CC, xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize
) {
    X_DEBUG_PRINTF(
        "ConnectionId=%" PRIx64 ", TargetAddress=%s, CommandId=%" PRIx32 "", CC.GetConnectionId(), CC.GetTargetAddress().ToString().c_str(), CommandId
    );
    return true;
}

void xPA_RelayServerListManager::StartDownloadRelayServerList() {
    if (!ConfigCenterConnectionIdList.size()) {
        return;
    }
    X_DEBUG_PRINTF("");

    ++DownloadRoundCounter;
    size_t R   = random() % ConfigCenterConnectionIdList.size();
    auto   CID = ConfigCenterConnectionIdList[R];
    auto   Q   = xCC_PA_GetRelayServerListVersion();

    PostMessage(CID, Cmd_CC_PA_GetRelayServerListVersion, DownloadRoundCounter, Q);
}
