#include "./relay_connection_manager.hpp"

static constexpr const int64_t RECONNECT_OR_KEEPALIVE_TIMEOUT         = 60'000;
static constexpr const int64_t KEEPALIVE_RESPONSE_TIMEOUT             = RECONNECT_OR_KEEPALIVE_TIMEOUT / 3 + RECONNECT_OR_KEEPALIVE_TIMEOUT;
static constexpr const int     MAX_RECONNECT_OR_KEEPALIVE_CHECK_COUNT = 200;

bool xPA_RelayConnectionManager::Init(xIoContext * IoContextPtr) {
    this->ICP = IoContextPtr;
    RuntimeAssert(ConnectionIdPool.Init(MAX_RELAY_SERVER_ID));
    Ticker.Update();
    return true;
}

void xPA_RelayConnectionManager::Clean() {
    for (auto & Entry : RelayGroupMap) {
        KillGroupList.GrabTail(*Steal(Entry.second));
    }
    Renew(RelayGroupMap);
    DoFreeKillGroupList();
    ConnectionIdPool.Clean();
}

void xPA_RelayConnectionManager::Tick(uint64_t NowMS) {
    Ticker.Update(NowMS);
    DoFreeKillList();
    DoFreeKillGroupList();
    // reconnect must run after Free***List, to ensure ConnectionIdPool has enough resources
    DoReconnectAndKeepAlive();
}

void xPA_RelayConnectionManager::AddRelayGroup(uint64_t RuntimeServerId, const xNetAddress & TargetAddress) {

    X_DEBUG_PRINTF("%" PRIx64 ": %s", RuntimeServerId, TargetAddress.ToString().c_str());
    // Never Call this in a callback !!!!
    assert(TargetAddress && TargetAddress.Port);
    auto & GP = RelayGroupMap[RuntimeServerId];
    if (GP) {
        if (GP->TargetAddress == TargetAddress) {
            return;
        }
        DeferKillGroup(Steal(GP));
    }

    GP                = new xPA_RelayGroup;
    GP->TargetAddress = TargetAddress;
    for (auto & RC : GP->Connections) {
        RC.GroupPtr = GP;
        // set this to zero to enhance preload new server group startup time
        RC.LastKeepAliveRequestTimestampMS = 0;
        IdleList.GrabHead(RC);
    }
}

void xPA_RelayConnectionManager::OnConnected(xTcpConnection * TcpConnectionPtr) {
    auto RCP = static_cast<xPA_RelayConnection *>(TcpConnectionPtr);
    X_DEBUG_PRINTF("ConnectionId=%" PRIx64 "", RCP->ConnectionId);
    KeepAlive(RCP);
}

void xPA_RelayConnectionManager::OnPeerClose(xTcpConnection * TcpConnectionPtr) {
    auto RCP = static_cast<xPA_RelayConnection *>(TcpConnectionPtr);
    X_DEBUG_PRINTF("ConnectionId=%" PRIx64 "", RCP->ConnectionId);
    DeferKillConnection(RCP);
}

size_t xPA_RelayConnectionManager::OnData(xTcpConnection * TcpConnectionPtr, ubyte * DataPtr, size_t DataSize) {
    auto RCP = static_cast<xPA_RelayConnection *>(TcpConnectionPtr);
    X_DEBUG_PRINTF("\n%s", HexShow(DataPtr, DataSize).c_str());

    size_t RemainSize = DataSize;
    while (RemainSize >= PacketHeaderSize) {
        auto Header = xPacketHeader::Parse(DataPtr);
        if (!Header) { /* header error */
            return InvalidDataSize;
        }
        auto PacketSize = Header.PacketSize;  // make a copy, so Header can be reused
        if (RemainSize < PacketSize) {        // wait for data
            break;
        }
        if (Header.IsKeepAlive()) {
            KeepAlive(RCP);
        } else {
            auto PayloadPtr  = xPacket::GetPayloadPtr(DataPtr);
            auto PayloadSize = Header.GetPayloadSize();
            if (!OnPacket(RCP, Header.CommandId, Header.RequestId, PayloadPtr, PayloadSize)) { /* packet error */
                return InvalidDataSize;
            }
        }
        DataPtr    += PacketSize;
        RemainSize -= PacketSize;
    }
    return DataSize - RemainSize;
}

void xPA_RelayConnectionManager::DoFreeKillList() {
    while (auto RCP = static_cast<xPA_RelayConnection *>(KillList.PopHead())) {
        ConnectionIdPool.Release(Steal(RCP->ConnectionId));
        RCP->Clean();

        RCP->LastKeepAliveRequestTimestampMS = Ticker();
        IdleList.GrabTail(*RCP);
    }
}

void xPA_RelayConnectionManager::DoFreeGroup(xPA_RelayGroup * GP) {
    for (auto & RC : GP->Connections) {
        if (RC.IsOpen()) {
            ConnectionIdPool.Release(Steal(RC.ConnectionId));
            RC.Clean();
        }
    }
    delete GP;
}

void xPA_RelayConnectionManager::KeepAlive(xPA_RelayConnection * RCP) {
    RCP->LastKeepAliveResponseTimestampMS = Ticker();
    IdleList.GrabTail(*RCP);
}

void xPA_RelayConnectionManager::DeferKillConnection(xPA_RelayConnection * RCP) {
    KillList.GrabTail(*RCP);
}

void xPA_RelayConnectionManager::DeferKillGroup(xPA_RelayGroup * GP) {
    KillGroupList.GrabTail(*GP);
}

void xPA_RelayConnectionManager::DoFreeKillGroupList() {
    // note: all nodes in this kill group list is removed from relay group map
    while (auto GP = static_cast<xPA_RelayGroup *>(KillGroupList.PopHead())) {
        DoFreeGroup(GP);
    }
}

void xPA_RelayConnectionManager::DoReconnectAndKeepAlive() {
    auto NowMS      = Ticker();
    auto TempList   = xList<xPA_RelayConnectionIdleNode>();
    auto CheckCount = (int)0;
    while (auto RCP = (xPA_RelayConnection *)IdleList.PopHead([=, &CheckCount](const xPA_RelayConnectionIdleNode & Node) -> bool {
        if (++CheckCount > MAX_RECONNECT_OR_KEEPALIVE_CHECK_COUNT) {
            return false;
        }
        return RECONNECT_OR_KEEPALIVE_TIMEOUT <= SignedDiff(NowMS, Node.LastKeepAliveRequestTimestampMS);
    })) {
        if (!RCP->IsOpen()) {
            RCP->ConnectionId = ConnectionIdPool.Acquire();
            RCP->Init(this->ICP, RCP->GroupPtr->TargetAddress, this);
        } else {
            if (KEEPALIVE_RESPONSE_TIMEOUT <= SignedDiff(NowMS, RCP->LastKeepAliveResponseTimestampMS)) {
                X_DEBUG_PRINTF("RemoveKeepAliveResponseTimeoutConnection ConnectionId=%" PRIx64 "", RCP->ConnectionId);
                DeferKillConnection(RCP);
                continue;
            }
            RCP->PostRequestKeepAlive();
        }
        RCP->LastKeepAliveRequestTimestampMS = NowMS;
        TempList.AddTail(*RCP);
    }
    IdleList.GrabListTail(TempList);
}

bool xPA_RelayConnectionManager::OnPacket(
    xPA_RelayConnection * RCP, xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize
) {
    X_DEBUG_PRINTF("ConnectionId=%" PRIx64 ", CommandId=%" PRIx32 "\n%s\n", RCP->ConnectionId, CommandId, HexShow(PayloadPtr, PayloadSize).c_str());
    return true;
}
