#include "./device_selector.hpp"

#include "./_global.hpp"

#include <pp_protocol/command.hpp>
#include <pp_protocol/internal/all.hpp>

void xDS_DeviceSelectorService::OnClientConnected(xServiceClientConnection & Connection) {
}

void xDS_DeviceSelectorService::OnClientClose(xServiceClientConnection & Connection) {
}

bool xDS_DeviceSelectorService::OnClientPacket(
    xServiceClientConnection & Connection, xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize
) {
    X_DEBUG_PRINTF("CommandId: %" PRIx64 ", \n%s", CommandId, HexShow(PayloadPtr, PayloadSize).c_str());

    switch (CommandId) {
        case Cmd_DSR_DS_DeviceOnline: {
            X_DEBUG_PRINTF("");
            auto PP = xPP_DeviceInfoUpdate();
            if (!PP.Deserialize(PayloadPtr, PayloadSize)) {
                X_DEBUG_PRINTF("Invalid device info");
                return true;
            }

            auto LocalDevInfo                  = xDR_DeviceInfoBase{};
            LocalDevInfo.DeviceId              = PP.DeviceUuid;
            LocalDevInfo.ReleayServerRuntimeId = PP.RelayServerRuntimeId;
            LocalDevInfo.DeviceRelaySideKey    = PP.RelaySideDeviceKey;

            LocalDevInfo.CountryId = PP.CountryId;
            LocalDevInfo.StateId   = PP.StateId;
            LocalDevInfo.CityId    = PP.CityId;

            DeviceContextManager.UpdateDevice(LocalDevInfo);
            break;
        }
        default:
            X_DEBUG_PRINTF("Invalid command id");
            return true;
    }
    return true;
}

void xDS_DeviceSelectorService::OnCleanupClientConnection(const xServiceClientConnection & Connection) {
}

/***
 *
 *
 *
 *
 */

bool xDS_DeviceObserver::OnServerPacket(xClientConnection & CC, xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) {
    switch (CommandId) {
        case Cmd_DeviceSelector_AcquireDevice:
            return OnSelectDevice(CC, RequestId, PayloadPtr, PayloadSize);

        default:
            break;
    }
    X_DEBUG_PRINTF("CommandId: %" PRIx64 ", \n%s", CommandId, HexShow(PayloadPtr, PayloadSize).c_str());
    return true;
}

bool xDS_DeviceObserver::OnSelectDevice(xClientConnection & CC, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) {
    auto Req = xPP_AcquireDevice();
    if (!Req.Deserialize(PayloadPtr, PayloadSize)) {
        X_DEBUG_PRINTF("Invalid protocol");
        return true;
    }

    auto PD = (const xDS_DeviceContext *)nullptr;
    if (Req.CityId) {
        PD = DeviceContextManager.SelectDeviceByCityId(Req.CityId);
    } else if (Req.StateId) {
        PD = DeviceContextManager.SelectDeviceByStateId(Req.StateId);
    } else if (Req.CountryId) {
        PD = DeviceContextManager.SelectDeviceByCountryId(Req.CountryId);
    }

    auto Resp = xPP_AcquireDeviceResp();
    if (PD) {
        Resp.DeviceRelayServerRuntimeId = PD->InfoBase.ReleayServerRuntimeId;
        Resp.DeviceRelaySideId          = PD->InfoBase.DeviceRelaySideKey;
    }

    PostMessage(CC, Cmd_DeviceSelector_AcquireDeviceResp, RequestId, Resp);
    return true;
}
