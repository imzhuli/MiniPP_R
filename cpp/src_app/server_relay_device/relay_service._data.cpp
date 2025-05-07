#include "./relay_service.hpp"

#include "./_global.hpp"
#include "./connection.hpp"

#include <algorithm>
#include <pp_protocol/command.hpp>
#include <pp_protocol/device_relay/connection.hpp>
#include <pp_protocol/device_relay/dns_query.hpp>
#include <pp_protocol/device_relay/init_ctrl_stream.hpp>
#include <pp_protocol/device_relay/init_data_stream.hpp>
#include <pp_protocol/device_relay/post_data.hpp>
#include <pp_protocol/proxy_relay/challenge.hpp>
#include <pp_protocol/proxy_relay/connection.hpp>

bool xDeviceRelayService::OnDataPacket(xRD_DeviceConnection * Conn, xPacketCommandId CommandId, xPacketRequestId RequestId, const ubyte * Payload, size_t PayloadSize) {
    X_DEBUG_PRINTF("Cmd=%" PRIx64 ", Request body: \n%s", CommandId, HexShow(Payload, PayloadSize).c_str());
    switch (CommandId) {
        case Cmd_DV_RL_InitDataStream: {
            return OnTerminalInitDataStream(Conn, RequestId, Payload, PayloadSize);
        }
        case Cmd_DV_RL_NotifyConnectionState: {
            return OnTerminalTargetConnectionUpdate(Conn, RequestId, Payload, PayloadSize);
        }
        case Cmd_DV_RL_PostData: {
            return OnTerminalPostData(Conn, RequestId, Payload, PayloadSize);
        }
        default:
            break;
    }
    return false;
}

bool xDeviceRelayService::OnTerminalInitDataStream(xRD_DeviceConnection * Conn, xPacketRequestId RequestId, const ubyte * Payload, size_t PayloadSize) {
    auto S = xInitDataStream();
    if (!S.Deserialize(Payload, PayloadSize)) {
        return false;
    }
    auto CtrlConn = DeviceConnectionManager.GetConnectionById(S.CtrlId);
    if (!CtrlConn) {
        X_DEBUG_PRINTF("no ctrl id conn found");
        return false;
    }
    if (CtrlConn->DeviceId) {
        X_DEBUG_PRINTF("duplicate device connection");
        return false;
    }

    auto NewDevice = DeviceManager.NewDevice();
    if (!NewDevice) {
        X_DEBUG_PRINTF("failed to create device context");
        DeviceConnectionManager.DeferReleaseConnection(CtrlConn);
        return false;
    }

    X_DEBUG_PRINTF("DataKey: %s", S.DataKey.c_str());
    auto Segs = Split(S.DataKey, ":");
    if (Segs.size() != 2 || Segs[1].size() != 24) {
        X_DEBUG_PRINTF("Invalid DataKey (missing geo info)");
        return false;
    }
    auto RawGeo = HexToStr(Segs[1]);

    auto R     = xInitDataStreamResp();
    R.Accepted = true;

    // accept data stream and move it to long idle list
    Conn->PostMessage(Cmd_DV_RL_InitDataStreamResp, RequestId, R);

    X_DEBUG_PRINTF("device accepted, DeviceRuntimeId:%" PRIx64 ", DevicdLocalIdString=%s", NewDevice->DeviceRuntimeId, S.DeviceLocalIdString.c_str());
    Conn->DeviceId                 = NewDevice->DeviceRuntimeId;
    CtrlConn->DeviceId             = NewDevice->DeviceRuntimeId;
    NewDevice->CtrlConnection      = CtrlConn;
    NewDevice->DataConnection      = Conn;
    NewDevice->DeviceLocalIdString = S.DeviceLocalIdString;

    auto GeoReader               = xStreamReader(RawGeo.data());
    NewDevice->GeoInfo.CountryId = GeoReader.R4();
    NewDevice->GeoInfo.StateId   = GeoReader.R4();
    NewDevice->GeoInfo.CityId    = GeoReader.R4();

    DeviceConnectionManager.KeepAlive(Conn);

    // test: send dns query:
    // auto DQ     = xDnsQuery();
    // DQ.Hostname = "www.163.com";
    // CtrlConn->PostMessage(Cmd_DV_RL_DnsQuery, 1024, DQ);

    DeviceManager.ReportDeviceState(NewDevice);
    return true;
}

bool xDeviceRelayService::OnTerminalTargetConnectionUpdate(xRD_DeviceConnection * Conn, xPacketRequestId RequestId, const ubyte * Payload, size_t PayloadSize) {
    auto S = xTR_ConnectionStateNotify();
    if (!S.Deserialize(Payload, PayloadSize)) {
        return false;
    }

    X_DEBUG_PRINTF(
        "New ConnectionState: %s terminalSideCid=%" PRIx32 ", relaySideCid=%" PRIx64 ", tR=%" PRIu64 ", tW=%" PRIu64 "", xTR_ConnectionStateNotify::GetStateName(S.NewState),
        S.DeviceSideConnectionId, S.RelaySideConnectionId, S.TotalReadBytes, S.TotalWrittenBytes
    );

    auto CR = RelayConnectionManager.GetConnectionById(S.RelaySideConnectionId);
    if (!CR || CR->RelaySideConnectionId != S.RelaySideConnectionId) {
        X_DEBUG_PRINTF("Connection not found: Id=%" PRIx64 "", S.RelaySideConnectionId);
        return true;
    }

    auto F = xPR_ConnectionStateNotify();
    switch (S.NewState) {
        case xTR_ConnectionStateNotify::STATE_ESTABLISHED:
            CR->DeviceSideConnectionId = S.DeviceSideConnectionId;

            F.NewState = xPR_ConnectionStateNotify::STATE_ESTABLISHED;
            break;
        case xTR_ConnectionStateNotify::STATE_UPDATE_TRANSFER:
            F.NewState = xPR_ConnectionStateNotify::STATE_UPDATE_TRANSFER;
            break;
        case xTR_ConnectionStateNotify::STATE_CLOSED:
            F.NewState = xPR_ConnectionStateNotify::STATE_CLOSED;
            break;
        default:
            X_DEBUG_PRINTF("Unrecognized state");
            return false;
    }
    F.ProxySideConnectionId = CR->ProxySideConnectionId;
    F.RelaySideConnectionId = CR->RelaySideConnectionId;
    F.ProxySideConnectionId = CR->ProxySideConnectionId;

    auto PAConn = ProxyConnectionManager.GetConnectionById(CR->ProxyId);
    if (!PAConn) {
        X_DEBUG_PRINTF("proxy not found");
        // TODO 删除此连接
        return true;
    }
    PAConn->PostMessage(Cmd_PA_RL_NotifyConnectionState, 0, F);
    return true;
}

bool xDeviceRelayService::OnTerminalPostData(xRD_DeviceConnection * Conn, xPacketRequestId RequestId, const ubyte * Payload, size_t PayloadSize) {
    auto S = xTR_PostData();
    if (!S.Deserialize(Payload, PayloadSize)) {
        return false;
    }

    X_DEBUG_PRINTF("terminalSideCid=%" PRIx32 ", relaySideCid=%" PRIx64 ", size=%zi", S.DeviceSideConnectionId, S.RelaySideConnectionId, S.PayloadView.size());
    auto CR = RelayConnectionManager.GetConnectionById(S.RelaySideConnectionId);
    if (!CR || CR->RelaySideConnectionId != S.RelaySideConnectionId) {
        X_DEBUG_PRINTF("Connection not found: Id=%" PRIx64 "", S.RelaySideConnectionId);
        return true;
    }

    auto PAConn = ProxyConnectionManager.GetConnectionById(CR->ProxyId);
    if (!PAConn) {
        X_DEBUG_PRINTF("proxy not found");
        return true;
    }

    auto Push                  = xPR_PushData();
    Push.RelaySideConnectionId = CR->RelaySideConnectionId;
    Push.ProxySideConnectionId = CR->ProxySideConnectionId;
    Push.PayloadView           = S.PayloadView;
    PAConn->PostMessage(Cmd_PA_RL_PostData, 0, Push);

    return true;
}
