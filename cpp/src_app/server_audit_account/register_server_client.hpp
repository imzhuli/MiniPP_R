#pragma once
#include "../lib_server_util/base.hpp"

#include <pp_common/_.hpp>

class xAA_xServerIdClient : public xServerIdClient {
public:
    using xServerIdClient::Clean;
    using xServerIdClient::Init;

protected:
    void OnServerIdUpdated(uint64_t NewServerId) override;
};

class xAA_RegisterServerClient : public xClient {
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
