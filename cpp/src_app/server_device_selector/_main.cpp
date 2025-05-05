#include "./_global.hpp"
#include "./device_context_manager.hpp"

#include <server_arch/service.hpp>

auto IC  = xIoContext();
auto ICG = xResourceGuard(IC);

struct xDR_Service : public xService {

    void OnClientConnected(xServiceClientConnection & Connection) {
    }
    void OnClientClose(xServiceClientConnection & Connection) {
    }
    bool OnPacket(xServiceClientConnection & Connection, xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) {
        return true;
    }
    void OnCleanupConnection(const xServiceClientConnection & Connection) {
    }
};
auto DRService = xDR_Service();

int main(int argc, char ** argv) {
    auto CL = xCommandLine(
        argc, argv,
        {
            { 'c', "config", "config-file", true },
        }
    );

    auto DRSG = xResourceGuard(DRService, &IC, BindAddress, MAX_RELAY_DEVICE_SERVER_SUPPORTED, true);
    RuntimeAssert(DRSG);

    while (true) {
        IC.LoopOnce();
        DRService.Tick();
    }
    return 0;
}
