#pragma once
#include <map>
#include <object/object.hpp>
#include <pp_common/base.hpp>
#include <server_arch/service.hpp>

class xCC_RelayServerService : public xService {

public:
    bool Init(xIoContext * ICP, const xNetAddress & BindAddressForRelayServer);
    void Clean();
    void Tick(uint64_t NowMS);

private:
    xIoContext * ICP = nullptr;
};