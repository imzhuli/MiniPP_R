#pragma once
#include "../base.hpp"

struct xPP_DownloadAuthCacheServerList : xBinaryMessage {
    void     SerializeMembers() override { W(Version); }
    void     DeserializeMembers() override { R(Version); }
    uint32_t Version;
};

struct xPP_DownloadAuthCacheServerListResp : xBinaryMessage {

    struct xServerInfo {
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

    uint32_t                 Version;
    std::vector<xServerInfo> ServerInfoList;
    //
};

struct xPP_DownloadAuditDeviceServerList : xBinaryMessage {
    void     SerializeMembers() override { W(Version); }
    void     DeserializeMembers() override { R(Version); }
    uint32_t Version;
};

struct xPP_DownloadAuditDeviceServerListResp : xBinaryMessage {

    struct xServerInfo {
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

        if (Count >= MAX_DEVICE_AUDIT_SERVER_COUNT) {
            GetReader()->SetError();
            return;
        }
        ServerInfoList.resize(Count);
        for (auto & I : ServerInfoList) {
            R(I.ServerId);
            R(I.ExportServerAddress);
        }
    }

    uint32_t                 Version;
    std::vector<xServerInfo> ServerInfoList;
    //
};

struct xPP_DownloadAuditAccountServerList : xBinaryMessage {
    void     SerializeMembers() override { W(Version); }
    void     DeserializeMembers() override { R(Version); }
    uint32_t Version;
};

struct xPP_DownloadAuditAccountServerListResp : xBinaryMessage {

    struct xServerInfo {
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

        if (Count >= MAX_DEVICE_AUDIT_SERVER_COUNT) {
            GetReader()->SetError();
            return;
        }
        ServerInfoList.resize(Count);
        for (auto & I : ServerInfoList) {
            R(I.ServerId);
            R(I.ExportServerAddress);
        }
    }

    uint32_t                 Version;
    std::vector<xServerInfo> ServerInfoList;
    //
};