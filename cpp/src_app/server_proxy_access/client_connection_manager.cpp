#include "./client_connection_manager.hpp"

bool xPA_ClientConnectionManager::Init(xIoContext * ICP, const xNetAddress & BindAddress, size_t ConnectionPoolSize) {
    RuntimeAssert(TcpServer.Init(ICP, BindAddress, this));
    RuntimeAssert(ConnectionPool.Init(ConnectionPoolSize));

    return true;
}

void xPA_ClientConnectionManager::Clean() {
}

void xPA_ClientConnectionManager::Tick() {
    Ticker.Update();
    OnTick();
}

void xPA_ClientConnectionManager::Tick(uint64_t NowMS) {
    Ticker.Update(NowMS);
    OnTick();
}
