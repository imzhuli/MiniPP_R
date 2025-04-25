#pragma once
#include "../base.hpp"

struct xCC_ALL_GetBackendServerList : xBinaryMessage {
    void SerializeMembers() override {
        W(TimestampMS);
    };
    void DeserializeMembers() override {
        R(TimestampMS);
    };
    uint64_t TimestampMS = 0;
};

struct xCC_ALL_GetBackendServerListResp : xBinaryMessage {
    void SerializeMembers() override {
        assert(AddressListPtr);
        auto & AddressList = *AddressListPtr;
        auto   Count       = (uint32_t)AddressList.size();
        W(Count);
        for (auto & A : AddressList) {
            W(A);
        }
    };

    void DeserializeMembers() override {
        assert(AddressListPtr);
        auto & AddressList = *AddressListPtr;
        auto   Count       = (uint32_t)0;
        R(Count);
        AddressList.resize(Count);
        for (auto & A : AddressList) {
            R(A);
        }
    };
    std::vector<xNetAddress> * AddressListPtr;
};