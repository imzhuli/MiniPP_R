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
        case Cmd_DeviceSelector_AcquireDevice:
            return OnSelectDevice(Connection, RequestId, PayloadPtr, PayloadSize);
        default:
            X_DEBUG_PRINTF("Invalid command id");
            return true;
    }
    return true;
}

void xDS_DeviceSelectorService::OnCleanupClientConnection(const xServiceClientConnection & Connection) {
}

bool xDS_DeviceSelectorService::OnSelectDevice(xServiceClientConnection & CC, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) {
    auto Req = xPP_AcquireDevice();
    if (!Req.Deserialize(PayloadPtr, PayloadSize)) {
        X_DEBUG_PRINTF("Invalid protocol");
        return true;
    }
    X_DEBUG_PRINTF("RequestGeoInfo: %u/%u/%u", (unsigned)Req.CountryId, (unsigned)Req.StateId, (unsigned)Req.CityId);

    auto PD = (const xDS_DeviceContext *)nullptr;
    if (Req.CityId) {
        X_DEBUG_PRINTF("by CityId");
        PD = DeviceContextManager.SelectDeviceByCityId(Req.CityId);
    } else if (Req.StateId) {
        X_DEBUG_PRINTF("by StateId");
        PD = DeviceContextManager.SelectDeviceByStateId(Req.StateId);
    } else if (Req.CountryId) {
        X_DEBUG_PRINTF("by CountryId");
        PD = DeviceContextManager.SelectDeviceByCountryId(Req.CountryId);
    } else {
        X_DEBUG_PRINTF("no device select condition");
    }

    auto Resp = xPP_AcquireDeviceResp();
    if (PD) {
        Resp.DeviceRelayServerRuntimeId = PD->InfoBase.ReleayServerRuntimeId;
        Resp.DeviceRelaySideId          = PD->InfoBase.RelaySideDeviceId;
        X_DEBUG_PRINTF("DeviceSelected: ServerId=%" PRIx64 ", DeviceId=%" PRIx64 "", Resp.DeviceRelayServerRuntimeId, Resp.DeviceRelaySideId);
    } else {
        X_DEBUG_PRINTF("No device found!");
    }

    PostMessage(CC, Cmd_DeviceSelector_AcquireDeviceResp, RequestId, Resp);
    return true;
}

/***
 *
 *
 *
 *
 */

bool xDS_DeviceObserver::OnServerPacket(xClientConnection & CC, xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) {
    switch (CommandId) {
        case Cmd_DSR_DS_DeviceUpdate: {
            X_DEBUG_PRINTF("");
            auto PP = xPP_DeviceInfoUpdate();
            if (!PP.Deserialize(PayloadPtr, PayloadSize)) {
                X_DEBUG_PRINTF("Invalid device info");
                return true;
            }

            if (!PP.IsOffline) {
                auto LocalDevInfo                  = xDR_DeviceInfoBase{};
                LocalDevInfo.DeviceId              = PP.DeviceUuid;
                LocalDevInfo.ReleayServerRuntimeId = PP.RelayServerRuntimeId;
                LocalDevInfo.RelaySideDeviceId     = PP.RelaySideDeviceKey;

                LocalDevInfo.CountryId = PP.CountryId;
                LocalDevInfo.StateId   = PP.StateId;
                LocalDevInfo.CityId    = PP.CityId;

                DeviceContextManager.UpdateDevice(LocalDevInfo);
            } else {
                DeviceContextManager.RemoveDeviceById(PP.DeviceUuid);
            }
            break;
        }

        default:
            break;
    }
    X_DEBUG_PRINTF("CommandId: %" PRIx64 ", \n%s", CommandId, HexShow(PayloadPtr, PayloadSize).c_str());
    return true;
}
