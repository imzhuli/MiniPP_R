#pragma once
#include "../base.hpp"

struct xPP_RegisterAuthCacheServer : xBinaryMessage {

    void SerializeMembers() override {
        W(ServerId);
        W(ExportServerAddress);
    }

    void DeserializeMembers() override {
        R(ServerId);
        R(ExportServerAddress);
    }

    uint64_t    ServerId;
    xNetAddress ExportServerAddress;
    //
};

struct xPP_RegisterAuditDeviceServer : xBinaryMessage {

    void SerializeMembers() override {
        W(ServerId);
        W(ExportServerAddress);
    }

    void DeserializeMembers() override {
        R(ServerId);
        R(ExportServerAddress);
    }

    uint64_t    ServerId;
    xNetAddress ExportServerAddress;

    //
};

struct xPP_RegisterAuditAccountServer : xBinaryMessage {

    void SerializeMembers() override {
        W(ServerId);
        W(ExportServerAddress);
    }

    void DeserializeMembers() override {
        R(ServerId);
        R(ExportServerAddress);
    }

    uint64_t    ServerId;
    xNetAddress ExportServerAddress;

    //
};

////////////////////

struct xPP_RegisterServerResponse : xBinaryMessage {

    void SerializeMembers() override { W(Accepted); }
    void DeserializeMembers() override { R(Accepted); }

    bool Accepted;
};
