#pragma once
#include "./command_line.hpp"

#include <pp_common/base.hpp>

extern xRuntimeEnv RuntimeEnv;
extern xLogger *   Logger;
extern xTicker     ServiceTicker;

struct xRuntimeEnvGuard final : xNonCopyable {
    xRuntimeEnvGuard(int argc, char ** argv);
    ~xRuntimeEnvGuard();
};

/////////////////////////
struct xServiceRequestContext;
class xServiceRequestContextPool;

class xServiceRequestContext : private xListNode {
    friend class xList<xServiceRequestContext>;

public:
    uint64_t          RequestId;
    uint64_t          RequestTimestampMS;
    mutable xVariable RequestContext;
    mutable xVariable RequestContextEx;
};
using xServiceRequestContextList = xList<xServiceRequestContext>;

class xServiceRequestContextPool {
public:
    bool Init(size_t PoolSize);
    void Clean();

    auto Acquire(xVariable RequestContext = {}, xVariable RequestContextEx = {}) -> const xServiceRequestContext *;
    auto CheckAndGet(uint64_t RequestId) -> const xServiceRequestContext *;
    void Release(const xServiceRequestContext * RCP);

    template <typename tCallback = void(const xServiceRequestContext *)>
    void RemoveTimeoutRequests(uint64_t TimeoutMS, tCallback && Callback = IgnoreTimeoutRequest) {
        auto KillTimepoint = ServiceTicker() - TimeoutMS;
        while (auto P = (const xServiceRequestContext *)TimeoutList.PopHead([KillTimepoint](const xServiceRequestContext & N) { return N.RequestTimestampMS <= KillTimepoint; })) {
            std::forward<tCallback>(Callback)(P);
            Pool.Release(P->RequestId);
        }
    }

private:
    static void IgnoreTimeoutRequest(const xServiceRequestContext *) {}

private:
    xIndexedStorage<xServiceRequestContext> Pool;
    xServiceRequestContextList              TimeoutList;
};
