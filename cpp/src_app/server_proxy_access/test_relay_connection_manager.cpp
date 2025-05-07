#include "./test_relay_connection_manager.hpp"

#include "./_global.hpp"

#include <pp_protocol/command.hpp>
#include <pp_protocol/proxy_relay/challenge.hpp>

void xPA_RelayClient::OnServerConnected() {
    auto C = xPR_Challenge();
    PostMessage(Cmd_PA_RL_Challenge, 0, C);
}

bool xPA_RelayClient::OnServerPacket(xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) {
    if (CommandId == Cmd_PA_RL_Challenge) {
        X_DEBUG_PRINTF("Relay client challenged done");
        return true;
    }

    X_DEBUG_PRINTF("RelayServer Command: %" PRIx32 "", CommandId);

    switch (CommandId) {
        case Cmd_PA_RL_NotifyConnectionState:
            GlobalClientConnectionManager.OnRelaySideConnectionStateChange(PayloadPtr, PayloadSize);
            break;
        case Cmd_PA_RL_DestroyConnection:
            GlobalClientConnectionManager.OnDestroyConnection(PayloadPtr, PayloadSize);
            break;
        case Cmd_PA_RL_PostData:
            GlobalClientConnectionManager.OnRelaySidePushData(PayloadPtr, PayloadSize);
            break;
        default:
            X_DEBUG_PRINTF("Ignored Command: %" PRIx32 "", CommandId);
            break;
    }

    return true;
}

////////////

void xPATest_RCM::Tick(uint64_t NowMS) {
    for (auto & [K, V] : ClientMap) {
        Ignore(K);
        V->Tick(NowMS);
    }
}

bool xPATest_RCM::AddRelayServer(uint64_t Id, const xNetAddress Addr) {
    auto & V = ClientMap[Id];
    if (V) {
        return false;
    }
    auto CP = new xPA_RelayClient();
    RuntimeAssert(CP->Init(ICP, Addr));
    V = CP;
    return true;
}

xPA_RelayClient * xPATest_RCM::GetConnectionById(uint64_t Id) {
    auto Iter = ClientMap.find(Id);
    if (Iter == ClientMap.end()) {
        return nullptr;
    }
    return Iter->second;
}
