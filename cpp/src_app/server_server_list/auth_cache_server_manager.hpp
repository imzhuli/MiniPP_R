#pragma once
#include <object/object.hpp>
#include <pp_common/base.hpp>
#include <server_arch/service.hpp>

class xCC_AuthCacheServerManager : xService {
public:
    static constexpr const size_t MAX_AUTH_CACHE_SERVER_COUNT = 1000;
    static_assert(MAX_AUTH_CACHE_SERVER_COUNT < xObjectIdManagerMini::MaxObjectId);

    bool Init(xIoContext * IoContextPtr, const xNetAddress & BindAddress);
    void Clean();

    uint32_t AddAuthCacheServer(uint64_t ConnectionId);
    uint64_t RemoveAuthCacheServer(uint32_t ServerId);

    bool OnClientPacket(xServiceClientConnection & Connection, const xPacketHeader & Header, ubyte * PayloadPtr, size_t PayloadSize) override;

    bool OnAllocAuthCacheServerId(xServiceClientConnection & Connection, uint64_t RequestId, ubyte * PayloadPtr, size_t PayloadSize);
    bool OnAllocDownloadDispatcherList(xServiceClientConnection & Connection, uint64_t RequestId, ubyte * PayloadPtr, size_t PayloadSize);

private:
    xObjectIdManagerMini IdManager;

    //
};
