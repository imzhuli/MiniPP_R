#include "./pa_config.hpp"

#include <pp_protocol/cc_pa/relay_info.hpp>
#include <pp_protocol/command.hpp>

bool xCC_PAConfigManager::Init(xIoContext * ICP, const xNetAddress & BindAddress) {
    RuntimeAssert(xService::Init(ICP, BindAddress));
    return true;
}

void xCC_PAConfigManager::Clean() {
}

bool xCC_PAConfigManager::OnClientPacket(xServiceClientConnection & Connection, const xPacketHeader & Header, ubyte * PayloadPtr, size_t PayloadSize) {
    switch (Header.CommandId) {
        case Cmd_CC_PA_GetRelayServerListVersion:
            return OnQueryRelayServerListVersion(Connection, Header, PayloadPtr, PayloadSize);
        case Cmd_CC_PA_DownloadRelayServerList:
            return OnDownloadRelayServerList(Connection, Header, PayloadPtr, PayloadSize);

        default:
            X_DEBUG_PRINTF("Unknonw request, CmdId=%" PRIx32 ", Payload=\n%s", Header.CommandId, HexShow(PayloadPtr, PayloadSize).c_str());
            break;
    }

    return true;
}

bool xCC_PAConfigManager::OnQueryRelayServerListVersion(
    xServiceClientConnection & Connection, const xPacketHeader & Header, ubyte * PayloadPtr, size_t PayloadSize
) {
    auto R = xCC_PA_GetRelayServerListVersion();
    if (!R.Deserialize(PayloadPtr, PayloadSize)) {
        return false;
    }
    X_DEBUG_PRINTF("OldVersion=%u", (unsigned)R.OldVersion);

    auto Resp    = xCC_PA_GetRelayServerListVersionResp();
    Resp.Version = 1;
    PostMessage(Connection, Cmd_CC_PA_GetRelayServerListVersionResp, Header.RequestId, Resp);
    return true;
}

bool xCC_PAConfigManager::OnDownloadRelayServerList(
    xServiceClientConnection & Connection, const xPacketHeader & Header, ubyte * PayloadPtr, size_t PayloadSize
) {
    if (!BuildTotalList()) {
        X_DEBUG_PRINTF("Failed to build local relay server protocol list");
        return true;
    }

    return true;
}

bool xCC_PAConfigManager::BuildTotalList() {
    return false;
}
