#include "./device_selector_manager.hpp"

#include "./_global.hpp"

#include <pp_protocol/command.hpp>
#include <pp_protocol/internal/device_selector.hpp>

void xPA_DeviceSelectorManager::PostDeviceRequest(const xPA_DeviceRequest & Request) {
    auto M = xPP_AcquireDevice();

    M.CountryId        = Request.CountryId;
    M.StateId          = Request.StateId;
    M.CityId           = Request.CityId;
    M.RequireIpv6      = Request.RequireIpv6;
    M.RequireUdp       = Request.RequireUdp;
    M.RequireRemoteDns = Request.RequireRemoteDns;

    xClientPool::PostMessage(Cmd_DeviceSelector_AcquireDevice, Request.ClientConnectionId, M);
}

bool xPA_DeviceSelectorManager::OnServerPacket(xClientConnection & CC, xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) {
    switch (CommandId) {
        case Cmd_DeviceSelector_AcquireDeviceResp:
            OnDeviceSelectorResult(RequestId, PayloadPtr, PayloadSize);
            break;

        default:
            break;
    }

    return true;
}

void xPA_DeviceSelectorManager::OnDeviceSelectorResult(xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) {
    auto M = xPP_AcquireDeviceResp();
    if (!M.Deserialize(PayloadPtr, PayloadSize)) {
        return;
    }

    auto LR                 = xPA_DeviceRequestResp();
    LR.ClientConnectionId   = RequestId;
    LR.RelayServerRuntimeId = M.DeviceRelayServerRuntimeId;
    LR.DeviceRelaySideId    = M.DeviceRelaySideId;
    GlobalClientConnectionManager.OnDeviceSelected(LR);
}
