#include "./service_common.hpp"

#include <mutex>

/// @brief Common code in ////////

xRuntimeEnv RuntimeEnv    = {};
xLogger *   Logger        = nullptr;
xTicker     ServiceTicker = {};

static void InitLogger() {
    RuntimeAssert(!Logger);
    Logger = new xBaseLogger();

    auto Path = RuntimeEnv.GetCachePath(RuntimeEnv.ProgramName + ".log");
    cout << "LoggerPath: " << Path << endl;

    RuntimeAssert(static_cast<xBaseLogger *>(Logger)->Init(Path.c_str(), false));
}

static void CleanLogger() {
    RuntimeAssert(Logger);
    static_cast<xBaseLogger *>(Logger)->Clean();
    delete Steal(Logger);
}

static auto Instance = (xServiceEnvGuard *){};
static auto EnvMutex = std::mutex();

xServiceEnvGuard::xServiceEnvGuard(int argc, char ** argv) {
    auto G = std::lock_guard(EnvMutex);
    RuntimeAssert(!Instance);

    RuntimeEnv = xRuntimeEnv::FromCommandLine(argc, argv);
    InitLogger();
    Instance = this;
}

xServiceEnvGuard::~xServiceEnvGuard() {
    auto G = std::lock_guard(EnvMutex);
    RuntimeAssert(this == Instance);

    CleanLogger();
    Reset(RuntimeEnv);
    Reset(Instance);
}

/////////////////////////

bool xServiceRequestContextPool::Init(size_t PoolSize) {
    return Pool.Init(PoolSize);
}
void xServiceRequestContextPool::Clean() {
    Pool.Clean();
    assert(TimeoutList.IsEmpty());
}

void xServiceRequestContextPool::RemoveTimeoutRequests(uint64_t TimeoutMS) {
    auto KillTimepoint = ServiceTicker() - TimeoutMS;
    while (auto P = (xServiceRequestContext *)TimeoutList.PopHead([KillTimepoint](const xServiceRequestContext & N) { return N.RequestTimestampMS <= KillTimepoint; })) {
        Pool.Release(P->RequestId);
    }
}

auto xServiceRequestContextPool::Acquire(uint64_t SourceConnectionId, uint64_t SourceRequestId) -> xServiceRequestContext * {
    auto Id = Pool.Acquire();
    if (!Id) {
        return nullptr;
    }
    auto & N             = Pool[Id];
    N.RequestId          = Id;
    N.RequestTimestampMS = ServiceTicker();

    N.SourceConnectionId = SourceConnectionId;
    N.SourceRequestId    = SourceRequestId;
    return &N;
}

void xServiceRequestContextPool::Release(uint64_t RequestId) {
}
