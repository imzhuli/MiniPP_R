#include "./client_udp_socket_manager.hpp"

bool xPA_UdpSocketManager::Init(xIoContext * IoContextPtr, const xNetAddress & BindAddress) {
    ICP            = IoContextPtr;
    UdpBindAddress = BindAddress;
    RuntimeAssert(UdpContextPool.Init(60000));

    return true;
}

void xPA_UdpSocketManager::Clean() {
    UdpContextPool.Clean();
    Renew(UdpBindAddress);
    Renew(ICP);
}
