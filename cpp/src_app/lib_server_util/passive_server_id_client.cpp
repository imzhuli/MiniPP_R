#include "./passive_server_id_client.hpp"

void xServerIdClient::OnServerConnected() {
}

bool xServerIdClient::OnServerPacket(xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) {

    return true;
}
