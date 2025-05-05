#include "./connection.hpp"

/***************
 * Connection
 */
bool xRD_ConnectionBase::PostPacket(xPacketCommandId CmdId, xPacketRequestId RequestId, xBinaryMessage & Message) {
    ubyte Buffer[MaxPacketSize];
    auto  PSize = WriteMessage(CmdId, RequestId, Buffer, Message);
    if (!PSize) {
        return false;
    }
    PostData(Buffer, PSize);
    return true;
}
