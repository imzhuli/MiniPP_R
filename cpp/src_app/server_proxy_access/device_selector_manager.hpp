#pragma once
#include "./_mindef.hpp"

class xPA_DeviceSelectorManager : public xClientPool {

public:
    using xClientPool::AddServer;
    using xClientPool::Clean;
    using xClientPool::Init;
    using xClientPool::Tick;

    void PostDeviceRequest(const xPA_DeviceRequest & Request);

protected:
    bool OnServerPacket(xClientConnection & CC, xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) override;

private:
    void OnDeviceSelectorResult(xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize);
};
