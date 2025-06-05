#include "./dispatcher_input.hpp"

bool xAD_CacheDispatcherInput::Init(xIoContext * ICP, const xNetAddress & Address) {
    RuntimeAssert(xService::Init(ICP, Address, DEFAULT_MAX_SERVER_CONNECTIONS, true));
    return true;
}

void xAD_CacheDispatcherInput::Clean() {
    xService::Clean();
}

void xAD_CacheDispatcherInput::Tick(uint64_t NowMS) {
    xService::Tick(NowMS);
}
