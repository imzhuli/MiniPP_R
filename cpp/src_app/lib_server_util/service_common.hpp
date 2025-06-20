#pragma once
#include "./command_line.hpp"

#include <pp_common/base.hpp>

extern xRuntimeEnv RuntimeEnv;
extern xLogger *   Logger;
extern xTicker     ServiceTicker;

struct xServiceEnvGuard final : xNonCopyable {
    xServiceEnvGuard(int argc, char ** argv);
    ~xServiceEnvGuard();
};

/////////////////////////
struct xServiceRequestContext;
class xServiceRequestContextPool;

class xServiceRequestContext : private xListNode {
    friend class xServiceRequestContextPool;
    friend class xList<xServiceRequestContext>;

public:
    uint64_t GetRequestId() const { return RequestId; }
    uint64_t GetSourceConnectionId() const { return SourceConnectionId; }
    uint64_t GetSourceRequestId() const { return SourceRequestId; }

private:
    uint64_t RequestId;
    uint64_t RequestTimestampMS;
    uint64_t SourceConnectionId;
    uint64_t SourceRequestId;
};
using xServiceRequestContextList = xList<xServiceRequestContext>;

class xServiceRequestContextPool {
public:
    bool Init(size_t PoolSize);
    void Clean();
    void RemoveTimeoutRequests(uint64_t TimeoutMS);

    auto Acquire(uint64_t SourceConnectionId, uint64_t SourceRequestId) -> xServiceRequestContext *;
    void Release(uint64_t RequestId);

private:
    xIndexedStorage<xServiceRequestContext> Pool;
    xServiceRequestContextList              TimeoutList;
};
