#pragma once
#include <map>
#include <network/tcp_connection.hpp>
#include <pp_common/base.hpp>

static constexpr const size_t MAX_RELAY_SERVER_COUNT           = 4000;
static constexpr const size_t MAX_RELAY_SERVER_ID              = MAX_RELAY_SERVER_COUNT;
static constexpr const size_t CONNECTION_COUNT_PER_RELAY_GROUP = 3;

struct xPA_RelayConnection;
struct xPA_RelayGroup;

struct xPA_RelayConnectionIdleNode : xListNode {
    uint64_t LastKeepAliveRequestTimestampMS  = 0;
    uint64_t LastKeepAliveResponseTimestampMS = 0;
};

struct xPA_RelayConnection
    : xPA_RelayConnectionIdleNode
    , xTcpConnection {
    //
    xPA_RelayGroup * GroupPtr;
    xIndexId         ConnectionId = 0;
    //
};

struct xPA_RelayGroupKillNode : xListNode {};

struct xPA_RelayGroup : xPA_RelayGroupKillNode {

    xNetAddress         TargetAddress;
    int                 NextPickIndex = 0;
    xPA_RelayConnection Connections[CONNECTION_COUNT_PER_RELAY_GROUP];
};

class xPA_RelayConnectionManager : xTcpConnection::iListener {
public:
    bool Init(xIoContext * IoContextPtr);
    void Clean();
    void AddRelayGroup(uint64_t RuntimeRelayServerId, const xNetAddress & TargetAddress);
    void Tick(uint64_t NowMS);

    inline xTcpConnection * GetConnectionById(uint64_t ConnectionId) {
        if (auto TCPP = ConnectionIdPool.CheckAndGet(ConnectionId)) {
            return *TCPP;
        }
        return nullptr;
    }
    xTcpConnection * GetConnectionByRelayServerAddress(const xNetAddress & TargetAddress);

protected:
    void DoFreeKillList();
    void DoFreeKillGroupList();
    void DoReconnectAndKeepAlive();
    void DoFreeGroup(xPA_RelayGroup * GP);

    void KeepAlive(xPA_RelayConnection * RCP);
    void DeferKillConnection(xPA_RelayConnection * RCP);
    void DeferKillGroup(xPA_RelayGroup * GP);

protected:
    // callback on connected, normally this is not needed to be handled
    void   OnConnected(xTcpConnection * TcpConnectionPtr) override;
    void   OnPeerClose(xTcpConnection * TcpConnectionPtr) override;
    size_t OnData(xTcpConnection * TcpConnectionPtr, ubyte * DataPtr, size_t DataSize) override;

protected:
    virtual bool OnPacket(xPA_RelayConnection * RCP, xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize);

private:
    xIoContext *                           ICP = nullptr;
    xTicker                                Ticker;
    xList<xPA_RelayConnectionIdleNode>     IdleList;
    xList<xPA_RelayConnectionIdleNode>     KillList;
    xList<xPA_RelayGroupKillNode>          KillGroupList;
    std::map<uint64_t, xPA_RelayGroup *>   RelayGroupMap;
    xIndexedStorage<xPA_RelayConnection *> ConnectionIdPool;
};