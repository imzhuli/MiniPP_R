#include "./device_selector.hpp"

void xDS_DeviceSelectorService::OnClientConnected(xServiceClientConnection & Connection) {
}

void xDS_DeviceSelectorService::OnClientClose(xServiceClientConnection & Connection) {
}

bool xDS_DeviceSelectorService::OnClientPacket(
    xServiceClientConnection & Connection, xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize
) {
    X_DEBUG_PRINTF("CommandId: %" PRIx64 ", \n%s", CommandId, HexShow(PayloadPtr, PayloadSize).c_str());
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
    X_DEBUG_PRINTF("CommandId: %" PRIx64 ", \n%s", CommandId, HexShow(PayloadPtr, PayloadSize).c_str());
    return true;
}