#pragma once
#include <pp_common/base.hpp>

struct xPA_AccountCheckNode : xListNode {
    uint64_t CheckAccountTimestampMS = 0;
};
struct xPA_KillClientConnectionNode : xListNode {
    uint64_t LingerKillTimestamp = 0;
};

using xPA_AccountCheckList         = xList<xPA_AccountCheckNode>;
using xPA_KIllClientConnectionList = xList<xPA_KillClientConnectionNode>;

struct xPA_ClientConnection
    : xTcpConnection
    , xPA_AccountCheckNode
    , xPA_KillClientConnectionNode {
    using xTcpConnection::Init;

    enum eProxyPhase : uint8_t {
        eUnknown = 0,

        eS5WaitForAuthInfo,
        eS5WaitForAccountExchange,
        eS5WaitForRelayCheckResult,
        eS5WaitForConnectionRequest,
        eS5WaitForConnectionEstablish,
        eS5ConnectionReady,

        eHttpChallenge,
        eHttpRawChallenge,
        eHttpRawWaitForAccountExchange,
        eHttpRawWaitForRelayCheckResult,
        eHttpRawWaitForConnectionEstablish,
        eHttpRawReady,

        eHttpNormalChallenge,
        eHttpNormalWaitForAccountExchange,
        eHttpNormalWaitForRelayCheckResult,
        eHttpNormalForConnectionEstablish,
        eHttpNormalReady,

        eLingerKill,
        eError,
    };

    uint64_t ConnectionId = 0;

    eProxyPhase Phase             = eUnknown;
    uint64_t    StartTimeMS       = 0;
    uint64_t    RelayConnectionId = 0;
    uint64_t    RelayDeviceId     = 0;
    uint64_t    RelaySessionId    = 0;

    std::string UserPass;

    struct {
        std::string TargetHost;
        uint16_t    TargetPort;
        std::string RebuiltHttpHeader;
        std::string HttpHeaderLine;
        std::string AccountName;
        std::string Password;
        size_t      ContentLengthLeft = 0;
    } Http;
};

class xPA_ClientConnectionManager
    : public xTcpServer::iListener
    , public xTcpConnection::iListener {
public:
    bool Init(xIoContext * ICP, const xNetAddress & BindAddress, size_t ConnectionPoolSize);
    void Clean();
    void Tick();
    void Tick(uint64_t NowMS);

protected:
    void OnTick();
    void LingerKillConnection(xPA_KillClientConnectionNode & Conn) {
        Conn.LingerKillTimestamp = Ticker();
        LingerKillConnectionList.GrabTail(Conn);
    }
    void KillConnection(xPA_KillClientConnectionNode & Conn) {
        KillConnectionList.GrabTail(Conn);
    }
    void CleanupConnection(xPA_ClientConnection & Conn);

protected:
    void   OnNewConnection(xTcpServer * TcpServerPtr, xSocket && NativeHandle) override;
    size_t OnData(xTcpConnection * TcpConnectionPtr, ubyte * DataPtr, size_t DataSize) override {
        return DataSize;
    }
    void OnPeerClose(xTcpConnection * TcpConnectionPtr) override {
        Pure();
    }
    //
protected:
    xTicker                               Ticker;
    xTcpServer                            TcpServer;
    xIndexedStorage<xPA_ClientConnection> ConnectionPool;

    xPA_AccountCheckList         AccountTimeoutList;
    xPA_KIllClientConnectionList LingerKillConnectionList;
    xPA_KIllClientConnectionList KillConnectionList;
};
