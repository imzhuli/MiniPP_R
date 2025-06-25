#include "./account_audit_server_list_client.hpp"

#include <pp_protocol/command.hpp>

void xDownloadAuditAccountServerListClient::SetUpdateInterval(uint64_t IntervalMS) {
    UpdateIntervalMS = IntervalMS < MIN_UPDATE_INTERVAL_MS ? MIN_UPDATE_INTERVAL_MS : IntervalMS;
}

void xDownloadAuditAccountServerListClient::OnTick(uint64_t NowMS) {
    if (NowMS - LastRequestTimestampMS < UpdateIntervalMS) {
        return;
    }
    PostDownloadAuditAccountServerListRequest();
}

void xDownloadAuditAccountServerListClient::PostDownloadAuditAccountServerListRequest() {
    auto R    = xPP_DownloadAuditAccountServerList();
    R.Version = ServerListVersion;
    PostMessage(Cmd_DownloadAuditAccountServerList, 0, R);
    LastRequestTimestampMS = GetTimestampMS();
}

void xDownloadAuditAccountServerListClient::OnServerConnected() {
    PostDownloadAuditAccountServerListRequest();
}

bool xDownloadAuditAccountServerListClient::OnServerPacket(xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) {
    if (CommandId != Cmd_DownloadAuditAccountServerListResp) {
        return false;
    }

    auto R = xPP_DownloadAuditAccountServerListResp();
    if (!R.Deserialize(PayloadPtr, PayloadSize)) {
        return false;
    }
    if (ServerListVersion == R.Version) {
        return true;
    }

    for (auto & S : R.ServerInfoList) {
        SortedServerInfoList.emplace_back(xServerInfo{
            .ServerId = S.ServerId,
            .Address  = S.ExportServerAddress,
        });
    }
    std::sort(SortedServerInfoList.begin(), SortedServerInfoList.end(), [](xServerInfo & lhs, xServerInfo & rhs) { return lhs.ServerId < rhs.ServerId; });
    ServerListVersion = R.Version;

    OnServerListUpdated();
    return true;
}
