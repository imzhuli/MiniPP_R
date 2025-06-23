#pragma once
#include "../base.hpp"

struct xPP_DownloadAuthCacheServerList : xBinaryMessage {

    void     SerializeMembers() override { W(Version); }
    void     DeserializeMembers() override { R(Version); }
    uint32_t Version;
};

struct xPP_DownloadAuthCacheServerListResp : xBinaryMessage {

    struct ServerInfo {
        uint64_t    ServerId;
        xNetAddress ExportServerAddress;
    };

    void SerializeMembers() override {
        W(Version);
        uint32_t Count = ServerInfoList.size();
        W(Count);
        for (auto & I : ServerInfoList) {
            W(I.ServerId);
            W(I.ExportServerAddress);
        }
    }

    void DeserializeMembers() override {
        R(Version);

        uint32_t Count = 0;
        R(Count);

        if (Count >= MAX_AUTH_CACHE_SERVER_COUNT) {
            GetReader()->SetError();
            return;
        }
        ServerInfoList.resize(Count);
        for (auto & I : ServerInfoList) {
            R(I.ServerId);
            R(I.ExportServerAddress);
        }
    }

    uint32_t                Version;
    std::vector<ServerInfo> ServerInfoList;
    //
};

struct xPP_DownloadDeviceAuditCollectorServerList : xBinaryMessage {

    struct ServerInfo {
        uint64_t    ServerId;
        xNetAddress ExportServerAddress;
    };

    void SerializeMembers() override {
        assert(ServerInfoList.size() < MAX_DEVICE_AUDIT_COLLECTOR_SERVER_COUNT);
        uint32_t Count = ServerInfoList.size();
        W(Count);
        for (auto & I : ServerInfoList) {
            W(I.ServerId);
            W(I.ExportServerAddress);
        }
    }

    void DeserializeMembers() override {
        uint32_t Count = 0;
        R(Count);

        if (Count >= MAX_DEVICE_AUDIT_COLLECTOR_SERVER_COUNT) {
            GetReader()->SetError();
            return;
        }
        ServerInfoList.resize(Count);
        for (auto & I : ServerInfoList) {
            R(I.ServerId);
            R(I.ExportServerAddress);
        }
    }

    std::vector<ServerInfo> ServerInfoList;

    //
};