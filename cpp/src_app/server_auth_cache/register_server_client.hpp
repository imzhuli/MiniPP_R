#pragma once
#include "../lib_server_util/base.hpp"

#include <pp_common/_.hpp>

class xAC_xServerIdClient : public xServerIdClient {
public:
    bool Init(xIoContext * ICP, xNetAddress TargetAddress);
    using xServerIdClient::Clean;

protected:
    void OnServerIdUpdated(uint64_t NewServerId) override;
};

class xAC_RegisterServerClient : public xClient {
public:
    using xClient::Init;
    void Clean();
    using xClient::Tick;

    void UpdateServerId(uint64_t ServerId);
    void PostServerId();
    void OnServerConnected() override;

private:
    uint64_t ServerId = 0;
};
