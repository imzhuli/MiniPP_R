#include "./dispatcher_input.hpp"

bool xAD_CacheDispatcherInput::Init(xIoContext * ICP, const xNetAddress & Address) {
    RuntimeAssert(xService::Init(ICP, Address, 1024));
    return true;
}

void xAD_CacheDispatcherInput::Clean() {
    xService::Clean();
}

void xAD_CacheDispatcherInput::Tick(uint64_t NowMS) {
    xService::Tick(NowMS);
}
