#include "./relay_connection_manager.hpp"

static constexpr const int64_t RECONNECT_OR_KEEPALIVE_TIMEOUT         = 60'000;
static constexpr const int     MAX_RECONNECT_OR_KEEPALIVE_CHECK_COUNT = 200;

bool xPA_RelayConnectionManager::Init(xIoContext * IoContextPtr) {
    this->ICP = IoContextPtr;
    RuntimeAssert(ConnectionIdPool.Init(MAX_RELAY_SERVER_ID));
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

void xPA_RelayConnectionManager::AddRelayGroup(const xNetAddress & TargetAddress) {
    // Never Call this in a callback !!!!
    assert(TargetAddress && TargetAddress.Port);
    auto & GP = RelayGroupMap[TargetAddress];
    if (GP) {
        return;
    }
    GP                = new xPA_RelayGroup;
    GP->TargetAddress = TargetAddress;
    for (auto & RC : GP->Connections) {
        RC.GroupPtr = GP;
        // set this to zero to enhance preload new server group startup time
        RC.LastKeepAliveTimestampMS = 0;
        IdleList.GrabHead(RC);
    }
}

void xPA_RelayConnectionManager::OnConnected(xTcpConnection * TcpConnectionPtr) {
    auto RCP = static_cast<xPA_RelayConnection *>(TcpConnectionPtr);
    X_DEBUG_PRINTF("ConnectionId=%" PRIx64 "", RCP->ConnectionId);
}

void xPA_RelayConnectionManager::OnPeerClose(xTcpConnection * TcpConnectionPtr) {
    auto RCP = static_cast<xPA_RelayConnection *>(TcpConnectionPtr);
    // put to reconnect queue end
    X_DEBUG_PRINTF("ConnectionId=%" PRIx64 "", RCP->ConnectionId);
    KillList.GrabTail(*RCP);
}

size_t xPA_RelayConnectionManager::OnData(xTcpConnection * TcpConnectionPtr, ubyte * DataPtr, size_t DataSize) {
    auto RCP = static_cast<xPA_RelayConnection *>(TcpConnectionPtr);
    cout << "Data On: " << RCP->ConnectionId << endl;
    cout << HexShow(DataPtr, DataSize) << endl;
    return DataSize;

    // size_t RemainSize = DataSize;
    // while (RemainSize >= PacketHeaderSize) {
    //     auto Header = xPacketHeader::Parse(DataPtr);
    //     if (!Header) { /* header error */
    //         return InvalidDataSize;
    //     }
    //     auto PacketSize = Header.PacketSize;  // make a copy, so Header can be reused
    //     if (RemainSize < PacketSize) {        // wait for data
    //         break;
    //     }
    //     if (Header.IsKeepAlive()) {
    //         RCP->LastKeepAliveTimestampMS = Ticker();
    //     } else {
    //         auto PayloadPtr  = xPacket::GetPayloadPtr(DataPtr);
    //         auto PayloadSize = Header.GetPayloadSize();
    //         if (!OnPacket(RCP, Header, PayloadPtr, PayloadSize)) { /* packet error */
    //             return InvalidDataSize;
    //         }
    //     }
    //     DataPtr    += PacketSize;
    //     RemainSize -= PacketSize;
    // }
    // return DataSize - RemainSize;
}

void xPA_RelayConnectionManager::DoFreeKillList() {
    while (auto RCP = static_cast<xPA_RelayConnection *>(KillList.PopHead())) {
        ConnectionIdPool.Release(Steal(RCP->ConnectionId));
        RCP->Clean();

        RCP->LastKeepAliveTimestampMS = Ticker();
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
        return RECONNECT_OR_KEEPALIVE_TIMEOUT <= SignedDiff(NowMS, Node.LastKeepAliveTimestampMS);
    })) {
        if (!RCP->IsOpen()) {
            RCP->ConnectionId = ConnectionIdPool.Acquire();
            RCP->Init(this->ICP, RCP->GroupPtr->TargetAddress, this);
        } else {
            RCP->PostRequestKeepAlive();
        }
        RCP->LastKeepAliveTimestampMS = NowMS;
        TempList.AddTail(*RCP);
    }
    IdleList.GrabListTail(TempList);
}

bool xPA_RelayConnectionManager::OnPacket(xPA_RelayConnection * RCP, const xPacketHeader & Header, ubyte * PayloadPtr, size_t PayloadSize) {
    return true;
}
