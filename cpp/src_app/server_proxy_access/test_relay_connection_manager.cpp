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

    // static constexpr const xPacketCommandId Cmd_PA_RL_Challenge             = Cmd_PA_RL_Base + 0x01;
    // static constexpr const xPacketCommandId Cmd_PA_RL_ChallengeResp         = Cmd_PA_RL_Base + 0x02;
    // static constexpr const xPacketCommandId Cmd_PA_RL_CreateConnection      = Cmd_PA_RL_Base + 0x03;
    // static constexpr const xPacketCommandId Cmd_PA_RL_DestroyConnection     = Cmd_PA_RL_Base + 0x04;
    // static constexpr const xPacketCommandId Cmd_PA_RL_NotifyConnectionState = Cmd_PA_RL_Base + 0x05;
    // static constexpr const xPacketCommandId Cmd_PA_RL_PostData              = Cmd_PA_RL_Base + 0x06;
    // static constexpr const xPacketCommandId Cmd_PA_RL_ProxyClientNotify     = Cmd_PA_RL_Base + 0x09;

    switch (CommandId) {
        case Cmd_PA_RL_NotifyConnectionState:
            GlobalClientConnectionManager.OnRelaySideConnectionStateChange(PayloadPtr, PayloadSize);
            break;
        case Cmd_PA_RL_DestroyConnection:
            GlobalClientConnectionManager.OnDestroyConnection(PayloadPtr, PayloadSize);
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
