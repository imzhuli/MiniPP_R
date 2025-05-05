#include "./relay_service.hpp"

#include "./_global.hpp"
#include "./connection.hpp"

#include <algorithm>
#include <pp_protocol/command.hpp>
#include <pp_protocol/dv_rl/connection.hpp>
#include <pp_protocol/dv_rl/dns_query.hpp>
#include <pp_protocol/dv_rl/init_ctrl_stream.hpp>
#include <pp_protocol/dv_rl/init_data_stream.hpp>
#include <pp_protocol/dv_rl/post_data.hpp>
#include <pp_protocol/proxy_relay/challenge.hpp>
#include <pp_protocol/proxy_relay/connection.hpp>

bool xDeviceRelayService::OnDataPacket(
    xRD_DeviceConnection * Conn, xPacketCommandId CommandId, xPacketRequestId RequestId, const ubyte * Payload, size_t PayloadSize
) {
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

    auto R     = xInitDataStreamResp();
    R.Accepted = true;

    // accept data stream and move it to long idle list
    Conn->PostPacket(Cmd_DV_RL_InitDataStreamResp, RequestId, R);

    X_DEBUG_PRINTF("device accepted, DeviceRuntimeId:%" PRIu64 ", DevicdLocalIdString=%s", NewDevice->DeviceRuntimeId, S.DeviceLocalIdString.c_str());
    Conn->DeviceId                 = NewDevice->DeviceRuntimeId;
    CtrlConn->DeviceId             = NewDevice->DeviceRuntimeId;
    NewDevice->CtrlConnection      = CtrlConn;
    NewDevice->DataConnection      = Conn;
    NewDevice->DeviceLocalIdString = S.DeviceLocalIdString;
    DeviceConnectionManager.KeepAlive(Conn);

    // test: send dns query:
    // auto DQ     = xDnsQuery();
    // DQ.Hostname = "www.163.com";
    // CtrlConn->PostPacket(Cmd_DV_RL_DnsQuery, 1024, DQ);

    return true;
}

bool xDeviceRelayService::OnTerminalTargetConnectionUpdate(xRD_DeviceConnection * Conn, xPacketRequestId RequestId, const ubyte * Payload, size_t PayloadSize) {
    auto S = xTR_ConnectionStateNotify();
    if (!S.Deserialize(Payload, PayloadSize)) {
        return false;
    }

    X_DEBUG_PRINTF(
        "New ConnectionState: %s terminalSideCid=%" PRIx32 ", relaySideCid=%" PRIx64 ", tR=%" PRIu64 ", tW=%" PRIu64 "",
        xTR_ConnectionStateNotify::GetStateName(S.NewState), S.DeviceSideConnectionId, S.RelaySideConnectionId, S.TotalReadBytes, S.TotalWrittenBytes
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
    F.RelaySideConnectionId = CR->RelaySideConnectionId;
    F.ProxySideConnectionId = CR->ProxySideConnectionId;

    auto PAConn = ProxyConnectionManager.GetConnectionById(CR->ProxyId);
    if (!PAConn) {
        X_DEBUG_PRINTF("proxy not found");
        // TODO 删除此连接
        return true;
    }
    PAConn->PostPacket(Cmd_PA_RL_NotifyConnectionState, 0, F);
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
    PAConn->PostPacket(Cmd_PA_RL_PostData, 0, Push);

    return true;
}
