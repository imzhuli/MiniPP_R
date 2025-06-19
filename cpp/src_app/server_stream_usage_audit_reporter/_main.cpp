
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
            ::LocalServerId = NewServerId;
            DumpLocalServerId(LocalServerIdFilename);
            ServerReady = true;
        }
    }
};
auto ServerIdClient = xSUAR_Service();

int main(int argc, char ** argv) {

    RuntimeEnv = xRuntimeEnv::FromCommandLine(argc, argv);
    cout << ToString(RuntimeEnv) << endl;

    LoadConfig();
    auto LogGuard = xScopeGuard(InitLogger, CleanLogger);

    LocalServerIdFilename = RuntimeEnv.CacheDir / (RuntimeEnv.ProgramName + ".local_service_id");
    auto SICG             = xResourceGuard(ServerIdClient, &IC, ServerIdCenterAddress, LocalServerIdFilename);

    Logger->I("ServiceStart, Init LocalServerId=%" PRIx64 "", ServerIdClient.GetLocalServerId());

    while (true) {
        Ticker.Update();
        IC.LoopOnce();
        TickAll(Ticker(), ServerIdClient);
    }

    return 0;
}
