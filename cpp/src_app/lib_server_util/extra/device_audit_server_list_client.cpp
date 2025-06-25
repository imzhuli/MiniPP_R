#include "./device_audit_server_list_client.hpp"

#include <pp_protocol/command.hpp>

void xDownloadAuditDeviceServerListClient::SetUpdateInterval(uint64_t IntervalMS) {
    UpdateIntervalMS = IntervalMS < MIN_UPDATE_INTERVAL_MS ? MIN_UPDATE_INTERVAL_MS : IntervalMS;
}

void xDownloadAuditDeviceServerListClient::OnTick(uint64_t NowMS) {
    if (NowMS - LastRequestTimestampMS < UpdateIntervalMS) {
        return;
    }
    PostDownloadAuditDeviceServerListRequest();
}

void xDownloadAuditDeviceServerListClient::PostDownloadAuditDeviceServerListRequest() {
    auto R    = xPP_DownloadAuditDeviceServerList();
    R.Version = ServerListVersion;
    PostMessage(Cmd_DownloadAuditDeviceServerList, 0, R);
    LastRequestTimestampMS = GetTimestampMS();
}

void xDownloadAuditDeviceServerListClient::OnServerConnected() {
    PostDownloadAuditDeviceServerListRequest();
}

bool xDownloadAuditDeviceServerListClient::OnServerPacket(xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) {
    if (CommandId != Cmd_DownloadAuditDeviceServerListResp) {
        return false;
    }

    auto R = xPP_DownloadAuditDeviceServerListResp();
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
