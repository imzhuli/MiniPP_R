#include "./auth_cache_server_list_client.hpp"

#include <pp_protocol/command.hpp>

void xDownloadAuthCacheServerListClient::SetUpdateInterval(uint64_t IntervalMS) {
    UpdateIntervalMS = IntervalMS < MIN_UPDATE_INTERVAL_MS ? MIN_UPDATE_INTERVAL_MS : IntervalMS;
}

void xDownloadAuthCacheServerListClient::OnTick(uint64_t NowMS) {
    if (NowMS - LastRequestTimestampMS < UpdateIntervalMS) {
        return;
    }
    PostDownloadAuthCacheServerListRequest();
}

void xDownloadAuthCacheServerListClient::PostDownloadAuthCacheServerListRequest() {
    auto R    = xPP_DownloadAuthCacheServerList();
    R.Version = AuthCacheServerListVersion;
    PostMessage(Cmd_DownloadAuthCacheServerList, 0, R);
    LastRequestTimestampMS = GetTimestampMS();
}

void xDownloadAuthCacheServerListClient::OnServerConnected() {
    PostDownloadAuthCacheServerListRequest();
}

bool xDownloadAuthCacheServerListClient::OnServerPacket(xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) {
    if (CommandId != Cmd_DownloadAuthCacheServerListResp) {
        return false;
    }

    auto R = xPP_DownloadAuthCacheServerListResp();
    if (!R.Deserialize(PayloadPtr, PayloadSize)) {
        return false;
    }
    if (AuthCacheServerListVersion == R.Version) {
        return true;
    }

    for (auto & S : R.ServerInfoList) {
        SortedServerInfoList.emplace_back(xServerInfo{
            .ServerId = S.ServerId,
            .Address  = S.ExportServerAddress,
        });
    }
    std::sort(SortedServerInfoList.begin(), SortedServerInfoList.end(), [](xServerInfo & lhs, xServerInfo & rhs) { return lhs.ServerId < rhs.ServerId; });
    AuthCacheServerListVersion = R.Version;

    OnServerListUpdated();
    return true;
}
