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
