#include "./dispatcher_output.hpp"

bool xAD_CacheDispatcherOutput::Init(xIoContext * ICP, const xNetAddress & Address) {
    RuntimeAssert(xService::Init(ICP, Address, DEFAULT_MAX_SERVER_CONNECTIONS, true));
    return true;
}

void xAD_CacheDispatcherOutput::Clean() {
    xService::Clean();
}

void xAD_CacheDispatcherOutput::Tick(uint64_t NowMS) {
    xService::Tick(NowMS);
}
