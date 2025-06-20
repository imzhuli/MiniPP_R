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

auto xServiceRequestContextPool::Acquire(xVariable RequestContext, xVariable RequestContextEx) -> const xServiceRequestContext * {
    auto Id = Pool.Acquire();
    if (!Id) {
        return nullptr;
    }
    auto & N             = Pool[Id];
    N.RequestId          = Id;
    N.RequestTimestampMS = ServiceTicker();
    TimeoutList.AddTail(N);

    N.RequestContext   = RequestContext;
    N.RequestContextEx = RequestContextEx;

    return &N;
}

auto xServiceRequestContextPool::CheckAndGet(uint64_t RequestId) -> const xServiceRequestContext * {
    return Pool.CheckAndGet(RequestId);
}

void xServiceRequestContextPool::Release(const xServiceRequestContext * RCP) {
    assert(Pool.CheckAndGet(RCP->RequestId) == RCP);
    Pool.Release(RCP->RequestId);
}
