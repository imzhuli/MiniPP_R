#pragma once
#include <pp_common/_.hpp>
#include <server_arch/client.hpp>

class xServerIdClient : public xel::xClient {
public:
    bool Init(xIoContext * ICP, xNetAddress TargetAddress, uint64_t FirstTryServerId = 0) {
        if (!xClient::Init(ICP, TargetAddress)) {
            return false;
        }
        LocalServerId = FirstTryServerId;
        return true;
    }
    void Clean() {
        Reset(LocalServerId);
        xClient::Clean();
    };
    using xClient::Tick;

protected:
    virtual void OnServerIdUpdated(uint64_t NewServerId) = 0;

private:
    void OnServerConnected() final override;
    bool OnServerPacket(xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) final override;

private:
    uint64_t LocalServerId = 0;
};
