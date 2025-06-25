#pragma once
#include <pp_common/base.hpp>
#include <pp_protocol/internal/all.hpp>

class xDownloadAuditAccountServerListClient : public xClient {
public:
    static constexpr const uint64_t MIN_UPDATE_INTERVAL_MS = 60'000;

    struct xServerInfo {
        uint64_t    ServerId;
        xNetAddress Address;
    };

    uint32_t     GetVersion() const { return ServerListVersion; }
    const auto & GetSortedServerInfoList() const { return SortedServerInfoList; }
    void         SetUpdateInterval(uint64_t IntervalMS = 60'000);

protected:
    virtual void OnServerListUpdated() {}

private:
    void OnTick(uint64_t NowMS) override;

    void OnServerConnected() override;
    bool OnServerPacket(xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) override;

    void PostDownloadAuditAccountServerListRequest();

protected:
    uint64_t                 UpdateIntervalMS       = MIN_UPDATE_INTERVAL_MS;
    uint64_t                 LastRequestTimestampMS = 0;
    uint32_t                 ServerListVersion      = 0;
    std::vector<xServerInfo> SortedServerInfoList;
    //
};