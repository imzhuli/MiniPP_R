#pragma once
#include <pp_common/_.hpp>
#include <server_arch/client.hpp>

class xServerIdClient : public xel::xClient {
public:
    using xClient::Clean;
    using xClient::Init;
    using xClient::Tick;

protected:
    void OnServerConnected() override;
    bool OnServerPacket(xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) override;

private:
    uint64_t LocalServerId = 0;
};
