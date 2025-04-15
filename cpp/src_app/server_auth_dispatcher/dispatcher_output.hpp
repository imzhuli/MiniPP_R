#pragma once
#include <pp_common/base.hpp>
#include <server_arch/service.hpp>

class xAD_CacheDispatcherOutput : xService {
public:
    bool Init(xIoContext * ICP, const xNetAddress & Address);
    void Clean();
    void Tick(uint64_t NowMS);

    xServiceClientConnection * SelectConnection();

private:
};