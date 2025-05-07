#pragma once
#include <compare>
#include <map>
#include <pp_common/base.hpp>
#include <pp_protocol/command.hpp>
#include <pp_protocol/proxy_relay/challenge.hpp>
#include <server_arch/client.hpp>

struct xPA_RelayClient : xClient {

    void OnServerConnected() override;
    bool OnServerPacket(xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) override;
    //
};

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

    bool              AddRelayServer(uint64_t Id, const xNetAddress Addr);
    xPA_RelayClient * GetConnectionById(uint64_t Id);

private:
    xIoContext *                          ICP;
    std::map<uint64_t, xPA_RelayClient *> ClientMap;
};
