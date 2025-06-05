#include "./backend_server_manager.hpp"

#include <pp_protocol/cc_all/backend_server_list.hpp>
#include <pp_protocol/command.hpp>

static constexpr const uint64_t RELOAD_TIMEOUT_MS            = 2 * 60'000;
static constexpr const size_t   MAX_BACKEND_SERVER_LIST_SIZE = 100;

bool xCC_BackendServerManager::Init(xIoContext * ICP, const xNetAddress & BindAddress, const std::string & ForcedBackendServerListFile) {
    RuntimeAssert(xService::Init(ICP, BindAddress, DEFAULT_MAX_SERVER_CONNECTIONS));
    RuntimeAssert(!ForcedBackendServerListFile.empty());
    ServerListFile = ForcedBackendServerListFile;

    return true;
}
void xCC_BackendServerManager::Clean() {
    xService::Clean();
    Reset(LastReloadTimestamp);
}

void xCC_BackendServerManager::Tick(uint64_t NowMS) {
    xService::Tick(NowMS);
    if (NowMS - LastReloadTimestamp <= RELOAD_TIMEOUT_MS) {
        return;
    }
    ReloadServerList();
    LastReloadTimestamp = NowMS;
}

void xCC_BackendServerManager::ReloadServerList() {
    auto NewList = std::vector<xNetAddress>();
    auto Lines   = FileToLines(ServerListFile);

    size_t Counter = 0;
    for (auto & L : Lines) {
        L = Trim(L);
        if (L.empty()) {
            continue;
        }
        auto A = xNetAddress::Parse(L);
        if (!A || !A.Port) {
            cerr << "Invalid backend server address" << endl;
            return;
        }
        NewList.push_back(A);
        X_DEBUG_PRINTF("Add BackendServer: %s", A.ToString().c_str());
        if (++Counter >= MAX_BACKEND_SERVER_LIST_SIZE) {
            cerr << "MAX_BACKEND_SERVER_LIST_SIZE reached" << endl;
            break;
        }
    }
    std::sort(NewList.begin(), NewList.end());
    ServerList = std::move(NewList);
}

bool xCC_BackendServerManager::OnClientPacket(
    xServiceClientConnection & Connection, xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize
) {
    switch (CommandId) {
        case Cmd_ALL_CC_DownloadBackendServerList:
            return OnDownloadBackendServerList(Connection, CommandId, RequestId);
        default:
            break;
    }
    return true;
}

bool xCC_BackendServerManager::OnDownloadBackendServerList(xServiceClientConnection & Connection, xPacketCommandId CommandId, xPacketRequestId RequestId) {
    auto Resp           = xCC_ALL_GetBackendServerListResp{};
    Resp.AddressListPtr = &ServerList;

    PostMessage(Connection, Cmd_ALL_CC_DownloadBackendServerListResp, RequestId, Resp);
    return true;
}
