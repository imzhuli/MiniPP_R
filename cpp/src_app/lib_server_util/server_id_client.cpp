#include "./server_id_client.hpp"

#include <pp_protocol/command.hpp>
#include <pp_protocol/internal/server_id.hpp>

void xServerIdClient::OnServerConnected() {
    auto Req             = xPP_AcquireServerId();
    Req.PreviousServerId = LocalServerId;
    PostMessage(Cmd_ALL_CC_AcquireServerId, 0, Req);
}

bool xServerIdClient::OnServerPacket(xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) {
    if (CommandId != Cmd_ALL_CC_AcquireServerIdResp) {
        return false;
    }

    auto Resp = xPP_AcquireServerIdResp();
    if (!Resp.Deserialize(PayloadPtr, PayloadSize)) {
        X_DEBUG_PRINTF("invalid packet");
        return false;
    }
    LocalServerId = Resp.NewServerId;
    OnServerIdUpdated(LocalServerId);
    return true;
}
