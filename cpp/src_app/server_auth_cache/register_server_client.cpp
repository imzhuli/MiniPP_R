#include "./register_server_client.hpp"

#include "./_global.hpp"

#include <pp_protocol/command.hpp>
#include <pp_protocol/internal/all.hpp>

void xAC_xServerIdClient::OnServerIdUpdated(uint64_t NewServerId) {
    RegisterServerClient.UpdateServerId(NewServerId);
}

void xAC_RegisterServerClient::Clean() {
    Reset(ServerId);
    xClient::Clean();
}

void xAC_RegisterServerClient::OnServerConnected() {
    if (ServerId) {
        PostServerId();
    }
}

void xAC_RegisterServerClient::UpdateServerId(uint64_t ServerId) {
    assert(ServerId);
    if (this->ServerId == ServerId) {
        return;
    }
    this->ServerId = ServerId;

    if (IsOpen()) {
        PostServerId();
    }
}

void xAC_RegisterServerClient::PostServerId() {
    assert(ServerId);
    auto Req                = xPP_RegisterAuthCacheServer();
    Req.ServerId            = ServerId;
    Req.ExportServerAddress = ExportServerAddress;

    PostMessage(Cmd_RegisterAuthCacheServer, 0, Req);
}
