#pragma once
#include "./base.hpp"

#include <vector>

struct xDownloadServerInfo : xBinaryMessage {

    void SerializeMembers() override {
        W(Version, ServerIndex);
    }

    void DeserializeMembers() override {
        R(Version, ServerIndex);
    }

    uint32_t Version;
    uint32_t ServerIndex;
    //
};

struct xDownloadServerInfoResp : xBinaryMessage {

    static constexpr const uint32_t NPOS = uint32_t(-1);

    struct xServerInfo {
        uint64_t    ServerKey;
        xNetAddress Address;
        std::string ExtraInfo;
    };

    void SerializeMembers() override {
        W(Version, NextRequestIndex, Count);
        for (auto & S : ServerList) {
            W(S.ServerKey, S.Address, S.ExtraInfo);
        }
    }

    void DeserializeMembers() override {
        R(Version, NextRequestIndex, Count);
        ServerList.resize(Count);
        for (auto & S : ServerList) {
            R(S.ServerKey, S.Address, S.ExtraInfo);
        }
    }

    uint32_t                 Version;
    uint32_t                 NextRequestIndex;
    uint32_t                 Count;
    std::vector<xServerInfo> ServerList;

    //
};