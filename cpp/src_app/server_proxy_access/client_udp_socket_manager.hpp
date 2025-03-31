#pragma once
#include <pp_common/base.hpp>

struct xPA_UdpContext {
    uint64_t         RelatedTcpConnectionId = 0;
    xPA_UdpContext * UdpContextPtr          = nullptr;
};

class xPA_UdpSocketManager {

public:
    bool Init(xIoContext * IoContextPtr, const xNetAddress & BindAddress = {});
    void Clean();

private:
    xIoContext * ICP = nullptr;

    //
    xNetAddress                     UdpBindAddress;
    xIndexedStorage<xPA_UdpContext> UdpContextPool;
};
