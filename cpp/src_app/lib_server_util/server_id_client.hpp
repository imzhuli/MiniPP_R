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
    bool Init(xIoContext * ICP, xNetAddress TargetAddress, const std::string & LocalServerIdFilename) {
        if (!xClient::Init(ICP, TargetAddress)) {
            return false;
        }
        LoadLocalServerId(LocalServerIdFilename);
        return true;
    }
    void Clean() {
        Reset(LocalServerId);
        xClient::Clean();
    };
    using xClient::Tick;

    uint64_t GetLocalServerId() const { return LocalServerId; }

    void LoadLocalServerId(const std::string & LocalServerIdFilename);
    void DumpLocalServerId(const std::string & LocalServerIdFilename);

protected:
    virtual void OnServerIdUpdated(uint64_t NewServerId) = 0;

private:
    void OnServerConnected() final override;
    bool OnServerPacket(xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) final override;

private:
    uint64_t LocalServerId = 0;
};
