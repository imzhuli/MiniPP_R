#pragma once
#include "./_mindef.hpp"

using xPA_AccountCheckList         = xList<xPA_AccountCheckNode>;
using xPA_IdleClientConnectionList = xList<xPA_IdleClientConnectionNode>;
using xPA_KIllClientConnectionList = xList<xPA_KillClientConnectionNode>;

struct xPA_ClientConnection
    : xTcpConnection
    , xPA_AccountCheckNode
    , xPA_IdleClientConnectionNode
    , xPA_KillClientConnectionNode {
    using xTcpConnection::Init;

    enum eProxyPhase : uint8_t {
        eUnknown = 0,

        eS5WaitForAuthInfo,
        eS5WaitForAccountExchange,
        eS5WaitForDeviceSelection,
        eS5WaitForConnectionRequest,
        eS5WaitForConnectionEstablish,
        eS5ConnectionReady,
        eS5WaitForConnectionClosed,

        eHttpChallenge,
        eHttpRawChallenge,
        eHttpRawWaitForAccountExchange,
        eHttpRawWaitForDeviceSelection,
        eHttpRawWaitForConnectionEstablish,
        eHttpRawReady,
        eHttpRawClosed,  // TODO

        eHttpNormalChallenge,
        eHttpNormalWaitForAccountExchange,
        eHttpNormalWaitForDeviceSelection,
        eHttpNormalForConnectionEstablish,
        eHttpNormalReady,
        eHttpNormalClosed,

        eLingerKill,
        eError,
    };

    uint64_t ConnectionId = 0;

    eProxyPhase Phase                 = eUnknown;
    uint64_t    StartTimeMS           = 0;
    uint64_t    RelayConnectionId     = 0;
    uint64_t    RelaySideDeviceId     = 0;
    uint64_t    RelaySideConnectionId = 0;

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

public:
    void OnAuthResult(uint64_t SourceClientConnectionId, const xPA_AuthResult * PR);
    void OnDeviceSelected(const xPA_DeviceRequestResp & Result);
    void OnOpenRemoteConnection(xPA_ClientConnection * CCP, const xNetAddress & Address, const std::string_view HostnameView);
    void OnRelaySideConnectionStateChange(const ubyte * PayloadPtr, size_t PayloadSize);
    void OnDestroyConnection(const ubyte * PayloadPtr, size_t PayloadSize);
    void OnRelaySidePushData(const ubyte * PayloadPtr, size_t PayloadSize);

protected:
    void OnTick();
    auto GetConnectionById(uint64_t Id) {
        return ConnectionPool.CheckAndGet(Id);
    }
    void LingerKill(xPA_KillClientConnectionNode & Conn) {
        Conn.LingerKillTimestamp = Ticker();
        LingerKillConnectionList.GrabTail(Conn);
    }
    void Kill(xPA_KillClientConnectionNode & Conn) {
        KillConnectionList.GrabTail(Conn);
    }
    void KeepAlive(xPA_IdleClientConnectionNode & Node) {
        Node.LastDataExchangeTimestampMS = Ticker();
        IdleConnectionList.GrabTail(Node);
    }
    void CleanupConnection(xPA_ClientConnection & Conn);

protected:
    void   OnNewConnection(xTcpServer * TcpServerPtr, xSocket && NativeHandle) override;
    size_t OnData(xTcpConnection * TcpConnectionPtr, ubyte * DataPtr, size_t DataSize) override;
    void   OnPeerClose(xTcpConnection * TcpConnectionPtr) override;

protected:
    // S5
    // from client
    size_t OnChallenge(xPA_ClientConnection * ConnectionPtr, const void * DataPtr, size_t DataSize);
    size_t OnS5Challenge(xPA_ClientConnection * ConnectionPtr, const void * DataPtr, size_t DataSize);
    size_t OnS5ClientAuth(xPA_ClientConnection * ConnectionPtr, void * DataPtr, size_t DataSize);
    size_t OnS5ConnectionRequest(xPA_ClientConnection * ConnectionPtr, void * DataPtr, size_t DataSize);
    size_t OnS5UploadData(xPA_ClientConnection * ConnectionPtr, void * DataPtr, size_t DataSize);
    // from back service
    void OnS5ClientAuthFinished(xPA_ClientConnection * ConnectionPtr, const xPA_AuthResult * ARP);

    // HttpRaw
    // from client
    size_t OnHttpChallenge(xPA_ClientConnection * ConnectionPtr, const void * DataPtr, size_t DataSize);
    size_t OnHttpRawChallenge(xPA_ClientConnection * ConnectionPtr, const void * DataPtr, size_t DataSize);
    size_t OnHttpRawUploadData(xPA_ClientConnection * CCP, void * VoidDP, size_t DataSize);
    // from back service
    void OnHttpRawAuthFinished(xPA_ClientConnection * ConnectionPtr, const xPA_AuthResult * ARP);

    //
protected:
    xTicker                               Ticker;
    xTcpServer                            TcpServer;
    xIndexedStorage<xPA_ClientConnection> ConnectionPool;

    xPA_AccountCheckList         AccountTimeoutList;
    xPA_IdleClientConnectionList IdleConnectionList;
    xPA_KIllClientConnectionList LingerKillConnectionList;
    xPA_KIllClientConnectionList KillConnectionList;
};
