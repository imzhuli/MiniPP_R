#pragma once
#include <map>
#include <pp_common/base.hpp>
#include <server_arch/client_pool.hpp>

class xPA_AuthRequestManager : xClientPool {

    void OnServerConnected(xClientConnection & PC) override;
    void OnServerClose(xClientConnection & PC) override;
    bool OnServerPacket(xClientConnection & PC, const xPacketHeader & Header, ubyte * PayloadPtr, size_t PayloadSize) override;
};
