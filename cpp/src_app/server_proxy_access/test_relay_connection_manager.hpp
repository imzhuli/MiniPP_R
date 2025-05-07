#pragma once
#include <compare>
#include <map>
#include <pp_common/base.hpp>
#include <server_arch/client.hpp>

class xPATest_RCM {
public:
    bool Init(xIoContext * ICP) {
        this->ICP = ICP;
        return true;
    }
    void Clean() {
        for (auto & [K, V] : ClientMap) {
            Ignore(K);
            V->Clean();
            delete V;
        }
        Reset(ClientMap);
        Reset(ICP);
    }
    void Tick(uint64_t NowMS);

    bool      AddRelayServer(uint64_t Id, const xNetAddress Addr);
    xClient * GetConnectionById(uint64_t Id);

private:
    xIoContext *                  ICP;
    std::map<uint64_t, xClient *> ClientMap;
};
