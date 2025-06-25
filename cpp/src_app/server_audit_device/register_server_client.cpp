#include "./register_server_client.hpp"

#include "./_global.hpp"

#include <pp_protocol/command.hpp>
#include <pp_protocol/internal/all.hpp>

bool xAD_xServerIdClient::Init(xIoContext * ICP, xNetAddress TargetAddress) {
    return xServerIdClient::Init(ICP, TargetAddress, RuntimeEnv.DefaultLocalServerIdFilePath);
}

void xAD_xServerIdClient::OnServerIdUpdated(uint64_t NewServerId) {
    Logger->I("OnServerIdUpdated: %" PRIx64 "", NewServerId);
    DumpLocalServerId(RuntimeEnv.DefaultLocalServerIdFilePath);
    RegisterServerClient.UpdateServerId(NewServerId);
}

void xAD_RegisterServerClient::Clean() {
    Reset(ServerId);
    xClient::Clean();
}

void xAD_RegisterServerClient::OnServerConnected() {
    if (ServerId) {
        PostServerId();
    }
}

void xAD_RegisterServerClient::UpdateServerId(uint64_t ServerId) {
    assert(ServerId);
    if (this->ServerId == ServerId) {
        return;
    }
    this->ServerId = ServerId;

    if (IsOpen()) {
        PostServerId();
    }
}

void xAD_RegisterServerClient::PostServerId() {
    assert(ServerId);
    auto Req                = xPP_RegisterAuditDeviceServer();
    Req.ServerId            = ServerId;
    Req.ExportServerAddress = ExportServerAddress;

    PostMessage(Cmd_RegisterAuditDeviceServer, 0, Req);
}
