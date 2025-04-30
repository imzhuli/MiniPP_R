#include "./client_connection_manager.hpp"

static constexpr const uint64_t MAX_ACCOUNT_CHECK_TIMEOUT          = 3'000;
static constexpr const uint64_t MAX_CONNECTION_LIGNER_KILL_TIMEOUT = 10'000;

bool xPA_ClientConnectionManager::Init(xIoContext * ICP, const xNetAddress & BindAddress, size_t ConnectionPoolSize) {
    RuntimeAssert(TcpServer.Init(ICP, BindAddress, this));
    RuntimeAssert(ConnectionPool.Init(ConnectionPoolSize));

    return true;
}

void xPA_ClientConnectionManager::Clean() {
    ConnectionPool.Clean();
    TcpServer.Clean();
}

void xPA_ClientConnectionManager::Tick() {
    Ticker.Update();
    OnTick();
}

void xPA_ClientConnectionManager::Tick(uint64_t NowMS) {
    Ticker.Update(NowMS);
    OnTick();
}

void xPA_ClientConnectionManager::OnTick() {
    // remove timeout account connections
    while (auto PC = static_cast<xPA_ClientConnection *>(AccountTimeoutList.PopHead([KTP = Ticker() - MAX_ACCOUNT_CHECK_TIMEOUT](const xPA_AccountCheckNode & N
                                                                                    ) { return N.CheckAccountTimestampMS < KTP; }))) {
        CleanupConnection(*PC);
    }

    // remove linger killed connections
    while (auto PC = static_cast<xPA_ClientConnection *>(LingerKillConnectionList.PopHead(
               [KTP = Ticker() - MAX_CONNECTION_LIGNER_KILL_TIMEOUT](const xPA_KillClientConnectionNode & N) { return N.LingerKillTimestamp < KTP; }
           ))) {
        CleanupConnection(*PC);
    }

    // remove killed connections
    while (auto PC = static_cast<xPA_ClientConnection *>(KillConnectionList.PopHead())) {
        CleanupConnection(*PC);
    }
}

void xPA_ClientConnectionManager::OnNewConnection(xTcpServer * TcpServerPtr, xSocket && NativeHandle) {
    auto CID = ConnectionPool.Acquire();
    if (!CID) {
        XelCloseSocket(NativeHandle);
        return;
    }
    auto & C = ConnectionPool[CID];
    if (!C.Init(TcpServerPtr->GetIoContextPtr(), std::move(NativeHandle), this)) {
        X_DEBUG_PRINTF("Failed to create new connection object");
        ConnectionPool.Release(CID);
        return;
    }
    C.ConnectionId            = CID;
    C.CheckAccountTimestampMS = Ticker();
    AccountTimeoutList.AddTail(C);
}

void xPA_ClientConnectionManager::CleanupConnection(xPA_ClientConnection & Conn) {
    assert(&Conn == ConnectionPool.CheckAndGet(Conn.ConnectionId));
    Conn.Clean();
    ConnectionPool.Release(Conn.ConnectionId);
}

size_t xPA_ClientConnectionManager::OnData(xTcpConnection * TcpConnectionPtr, ubyte * DataPtr, size_t DataSize) {
    auto CP = (xPA_ClientConnection *)TcpConnectionPtr;
    switch (CP->Phase) {
        case xPA_ClientConnection::eUnknown:
            return OnChallenge(CP, DataPtr, DataSize);
            // case xProxyClientConnection::eS5WaitForAuthInfo:
            //     return OnS5ClientAuth(ConnectionPtr, DataPtr, DataSize);
            // case xProxyClientConnection::eS5WaitForConnectionRequest:
            //     return OnS5ConnectionRequest(ConnectionPtr, DataPtr, DataSize);
            // case xProxyClientConnection::eS5ConnectionReady:
            //     return OnS5UploadData(ConnectionPtr, DataPtr, DataSize);

            // case xProxyClientConnection::eHttpRawChallenge:
            //     return OnHttpRawChallenge(ConnectionPtr, DataPtr, DataSize);
            // case xProxyClientConnection::eHttpRawReady:
            //     return OnHttpRawUploadData(ConnectionPtr, DataPtr, DataSize);

            // case xProxyClientConnection::eHttpNormalChallenge:
            //     return OnHttpNormalChallenge(ConnectionPtr, DataPtr, DataSize);
            // case xProxyClientConnection::eHttpNormalReady:
            //     return OnHttpNormalUploadData(ConnectionPtr, DataPtr, DataSize);

        default:
            X_DEBUG_PRINTF("Unknown processed phase, closing connection");
            break;
    }
    Kill(*CP);
    return 0;
}

size_t xPA_ClientConnectionManager::OnChallenge(xPA_ClientConnection * ConnectionPtr, const void * DataPtr, size_t DataSize) {
    // minimal challenge header:
    if (DataSize < 3) {
        return 0;
    }
    if (((const ubyte *)DataPtr)[0] == 0x05) {  // version : S5
        return OnS5Challenge(ConnectionPtr, DataPtr, DataSize);
    }
    return OnHttpChallenge(ConnectionPtr, DataPtr, DataSize);
}

size_t xPA_ClientConnectionManager::OnS5Challenge(xPA_ClientConnection * ConnectionPtr, const void * DataPtr, size_t DataSize) {

    auto R = xStreamReader(DataPtr);
    R.Skip(1);  // skip type check bytes

    size_t NM = R.R1();  // number of methods
    if (!NM) {
        Kill(*ConnectionPtr);
        return 0;
    }
    size_t HeaderSize = 2 + NM;
    if (DataSize < HeaderSize) {
        return 0;
    }
    bool UserPassSupport = false;
    for (size_t i = 0; i < NM; ++i) {
        uint8_t Method = R.R1();
        if (Method == 0x02) {
            UserPassSupport = true;
            break;
        }
    }
    if (!UserPassSupport) {
        Kill(*ConnectionPtr);
        return 0;
    }
    ubyte Socks5Auth[2] = { 0x05, 0x02 };
    ConnectionPtr->PostData(Socks5Auth, sizeof(Socks5Auth));

    ConnectionPtr->Phase = xPA_ClientConnection::eS5WaitForAuthInfo;
    return HeaderSize;
}

size_t xPA_ClientConnectionManager::OnHttpChallenge(xPA_ClientConnection * ConnectionPtr, const void * DataPtr, size_t DataSize) {
    return InvalidDataSize;
}