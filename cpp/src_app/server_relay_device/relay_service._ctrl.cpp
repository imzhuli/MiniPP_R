#include "./relay_service.hpp"

#include "./_global.hpp"
#include "./connection.hpp"

#include <algorithm>
#include <pp_protocol/command.hpp>
#include <pp_protocol/device_relay/connection.hpp>
#include <pp_protocol/device_relay/init_ctrl_stream.hpp>
#include <pp_protocol/device_relay/init_data_stream.hpp>
#include <pp_protocol/device_relay/post_data.hpp>
#include <pp_protocol/proxy_relay/challenge.hpp>

bool xDeviceRelayService::OnCtrlPacket(xRD_DeviceConnection * Conn, xPacketHeader & Header, const ubyte * Payload, size_t PayloadSize) {
    X_DEBUG_PRINTF("Cmd=%" PRIx64 ", Request body: \n%s", Header.CommandId, HexShow(Payload, PayloadSize).c_str());
    switch (Header.CommandId) {
        case Cmd_Terminal_RL_InitCtrlStream: {
            return OnTerminalInitCtrlStream(Conn, Header, Payload, PayloadSize);
        }
    }
    return false;
}

bool xDeviceRelayService::OnTerminalInitCtrlStream(xRD_DeviceConnection * Conn, xPacketHeader & Header, const ubyte * Payload, size_t PayloadSize) {
    auto S = xInitCtrlStream();
    if (!S.Deserialize(Payload, PayloadSize)) {
        return false;
    }
    // TODO: check
    X_DEBUG_PRINTF("New Terminal Device: Ipv4:%s, Ipv6:%s", S.Ipv4Address.IpToString().c_str(), S.Ipv6Address.IpToString().c_str());

    auto R      = xInitCtrlStreamResp();
    R.DeviceId  = 0;
    R.CtrlId    = Conn->ConnectionId;
    R.DeviceKey = "hello world!";
    Conn->PostPacket(Cmd_Terminal_RL_InitCtrlStreamResp, Header.RequestId, R);
    DeviceConnectionManager.KeepAlive(Conn);
    return true;
}
