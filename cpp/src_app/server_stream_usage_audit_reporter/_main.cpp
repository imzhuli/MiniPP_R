
#include "./_global.hpp"

#include <pp_common/_.hpp>

static auto IC                    = xIoContext();
static auto ICG                   = xResourceGuard(IC);
static auto LocalServerIdFilename = std::string();
static auto ServerReady           = false;

struct xSUAR_Service : xServerIdClient {
    void OnServerIdUpdated(uint64_t NewServerId) {
        Logger->I("UpdateLocalServerId: %" PRIx64 "", NewServerId);
        if (NewServerId) {
            DumpLocalServerId(RuntimeEnv.DefaultLocalServerIdFilePath);
            ServerReady = true;
        }
    }
};
auto ServerIdClient = xSUAR_Service();

int main(int argc, char ** argv) {

    auto SEG = xRuntimeEnvGuard(argc, argv);

    LoadConfig();
    auto SICG = xResourceGuard(ServerIdClient, &IC, ServerIdCenterAddress, RuntimeEnv.DefaultLocalServerIdFilePath);

    Logger->I("ServiceStart, Init LocalServerId=%" PRIx64 "", ServerIdClient.GetLocalServerId());

    while (true) {
        ServiceTicker.Update();
        IC.LoopOnce();
        TickAll(ServiceTicker(), ServerIdClient);
    }

    return 0;
}
