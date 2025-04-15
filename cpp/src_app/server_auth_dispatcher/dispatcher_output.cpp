#include "./dispatcher_output.hpp"

bool xAD_CacheDispatcherOutput::Init(xIoContext * ICP, const xNetAddress & Address) {
    RuntimeAssert(xService::Init(ICP, Address, 1024));
    return true;
}

void xAD_CacheDispatcherOutput::Clean() {
    xService::Clean();
}

void xAD_CacheDispatcherOutput::Tick(uint64_t NowMS) {
    xService::Tick(NowMS);
}
