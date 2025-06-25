#pragma once
#include <pp_common/base.hpp>
#include <pp_protocol/internal/all.hpp>

class xDownloadAuthCacheServerListClient : public xClient {
public:
    static constexpr const uint64_t MIN_UPDATE_INTERVAL_MS = 60'000;

    struct xServerInfo {
        uint64_t    ServerId;
        xNetAddress Address;
    };

    uint32_t     GetVersion() const { return AuthCacheServerListVersion; }
    const auto & GetSortedServerInfoList() const { return SortedServerInfoList; }
    void         SetUpdateInterval(uint64_t IntervalMS = 60'000);

protected:
    virtual void OnServerListUpdated() {
        // for (auto & SI : ServerInfoList) {
        //     Logger->I("ServerId: %" PRIx64 ", ServerAddress: %s", SI.ServerId, SI.ExportServerAddress.ToString().c_str());
        // }
        // Logger->I("server list version updated: %" PRIu32 "", AuthCacheServerListVersion);
    }

private:
    void OnTick(uint64_t NowMS) override;

    void OnServerConnected() override;
    bool OnServerPacket(xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) override;

    void PostDownloadAuthCacheServerListRequest();

private:
    uint64_t                 UpdateIntervalMS           = MIN_UPDATE_INTERVAL_MS;
    uint64_t                 LastRequestTimestampMS     = 0;
    uint32_t                 AuthCacheServerListVersion = 0;
    std::vector<xServerInfo> SortedServerInfoList;
    //
};