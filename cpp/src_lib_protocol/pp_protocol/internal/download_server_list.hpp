#pragma once
#include "../base.hpp"

struct xPP_DownloadAuthCacheServerList : xBinaryMessage {

    static constexpr const uint32_t MAX_SERVER_SIZE = 150;

    struct ServerInfo {
        uint64_t    ServerId;
        xNetAddress ExportServerAddress;
    };

    void SerializeMembers() override {
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

        if (Count >= MAX_SERVER_SIZE) {
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

struct xPP_DownloadDeviceAuditCollectorServerList : xBinaryMessage {

    static constexpr const uint32_t MAX_SERVER_SIZE = 100;

    struct ServerInfo {
        uint64_t    ServerId;
        xNetAddress ExportServerAddress;
    };

    void SerializeMembers() override {
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

        if (Count >= MAX_SERVER_SIZE) {
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