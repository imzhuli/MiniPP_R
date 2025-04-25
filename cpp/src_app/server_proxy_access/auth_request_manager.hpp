#pragma once
#include "../lib_backend_connection/backend_connection_pool.hpp"

#include <map>
#include <pp_common/base.hpp>

class xPA_AuthRequestManager : public xClientPool {

public:
    bool Init(xIoContext * ICP, size_t MaxConnectionCount = 1024);
    void Clean();

protected:
    uint64_t PostAuthRequest();
    bool     OnBackendPacket(const xPacketHeader & Header, ubyte * PayloadPtr, size_t PayloadSize);

    //
};
