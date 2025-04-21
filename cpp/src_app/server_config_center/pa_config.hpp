#pragma once
#include <pp_common/base.hpp>
#include <server_arch/service.hpp>

class xCC_PAConfigManager : public xService {
public:
    bool Init(xIoContext * ICP, const xNetAddress & BindAddress);
    void Clean();

    bool OnPacket(xServiceClientConnection & Connection, const xPacketHeader & Header, ubyte * PayloadPtr, size_t PayloadSize) override;

public:
    bool OnQueryRelayServerListVersion(xServiceClientConnection & Connection, const xPacketHeader & Header, ubyte * PayloadPtr, size_t PayloadSize);
    bool OnDownloadRelayServerList(xServiceClientConnection & Connection, const xPacketHeader & Header, ubyte * PayloadPtr, size_t PayloadSize);
};
