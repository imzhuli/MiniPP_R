#pragma once
#include "../base.hpp"

struct xAD_BK_DeviceInfo {

    uint32_t    Version;
    std::string DeviceUuid;
    std::string RelayServerUuid;
    xNetAddress PrimaryIpv4Address;
    xNetAddress PrimaryIpv6Address;
    bool IsOffline;  // 下线时会设置这个标志位. 但考虑到服务器的变更, 不是所有设备都有上/下线标志, 后台服务器应当将30分钟未发统计的设备设为下线.
    bool SupportUdpChannel;
    bool SupportDnsRequests;
    bool SpeedLimitEnabled;

    uint64_t TotalOnlineTimeMS;  // 从上线时算起, 假设中间出现了统计丢失 (如后台认为30分钟下线的设备, 但后来又收到此数值比较大,
                                 // 可由后台简单处理, 比如理解成两个相同DeviceUuid平行时空的设备, 分别上线和下线, 因为实际业务中没必要处理此冲突)
    uint32_t TotalNewConnectionsSinceLastPost;
    uint32_t TotalClosedConnectionSinceLastPost;
    uint32_t TotalNewUdpChannelSinceLastPost;
    uint32_t TotalClosedUdpChannelSinceLastPost;
    uint32_t TotalDnsRequestSinceLastPost;

    uint64_t TotalUploadSizeSinceOnline;    // 指发向目标的数据
    uint64_t TotalDownloadSizeSinceOnline;  // 下载数据

    uint32_t CurrentConnectionCount;
    uint32_t CurrentUdpChannelCount;
    //
};

struct xAD_BK_ReportDeviceInfoList : xBinaryMessage {

    static constexpr const size16_t MAX_DEVICE_INFO_COUNT_PER_REPORT = 20;

    void SerializeMembers() override {
        size16_t Count = DeviceInfoList.size();
        assert(Count < MAX_DEVICE_INFO_COUNT_PER_REPORT);
        for (auto & D : DeviceInfoList) {
            W(D.Version);
            W(D.DeviceUuid);
            W(D.RelayServerUuid);
            W(D.PrimaryIpv4Address);
            W(D.PrimaryIpv6Address);

            W(D.IsOffline);
            W(D.SupportUdpChannel);
            W(D.SupportDnsRequests);
            W(D.SpeedLimitEnabled);

            W(D.TotalOnlineTimeMS);
            W(D.TotalNewConnectionsSinceLastPost);
            W(D.TotalClosedConnectionSinceLastPost);
            W(D.TotalNewUdpChannelSinceLastPost);
            W(D.TotalDnsRequestSinceLastPost);

            W(D.TotalUploadSizeSinceOnline);
            W(D.TotalDownloadSizeSinceOnline);
        }
    };

    void DeserializeMembers() override {
        size16_t Count = 0;
        R(Count);
        if (Count > MAX_DEVICE_INFO_COUNT_PER_REPORT) {
            xBinaryMessageReader::SetError();
            return;
        }
        DeviceInfoList.resize(Count);
        for (auto & D : DeviceInfoList) {
            R(D.Version);
            R(D.DeviceUuid);
            R(D.RelayServerUuid);
            R(D.PrimaryIpv4Address);
            R(D.PrimaryIpv6Address);

            R(D.IsOffline);
            R(D.SupportUdpChannel);
            R(D.SupportDnsRequests);
            R(D.SpeedLimitEnabled);

            R(D.TotalOnlineTimeMS);
            R(D.TotalNewConnectionsSinceLastPost);
            R(D.TotalClosedConnectionSinceLastPost);
            R(D.TotalNewUdpChannelSinceLastPost);
            R(D.TotalDnsRequestSinceLastPost);

            R(D.TotalUploadSizeSinceOnline);
            R(D.TotalDownloadSizeSinceOnline);
        }
    };

    std::vector<xAD_BK_DeviceInfo> DeviceInfoList;
};
