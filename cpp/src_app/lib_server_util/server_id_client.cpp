#include "./server_id_client.hpp"

#include <fstream>
#include <pp_protocol/command.hpp>
#include <pp_protocol/internal/server_id.hpp>

void xServerIdClient::OnServerConnected() {
    auto Req             = xPP_AcquireServerId();
    Req.PreviousServerId = LocalServerId;
    PostMessage(Cmd_ALL_CC_AcquireServerId, 0, Req);
}

bool xServerIdClient::OnServerPacket(xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) {
    if (CommandId != Cmd_ALL_CC_AcquireServerIdResp) {
        return false;
    }

    auto Resp = xPP_AcquireServerIdResp();
    if (!Resp.Deserialize(PayloadPtr, PayloadSize)) {
        X_DEBUG_PRINTF("invalid packet");
        return false;
    }
    LocalServerId = Resp.NewServerId;
    OnServerIdUpdated(LocalServerId);
    return true;
}

void xServerIdClient::LoadLocalServerId(const std::string & LocalServerIdFilename) {
    auto File  = LocalServerIdFilename;
    auto FSOpt = FileToStr(File);
    if (!FSOpt) {
        LocalServerId = 0;
        return;
    }
    LocalServerId = (uint64_t)strtoumax(FSOpt->c_str(), nullptr, 10);
}

void xServerIdClient::DumpLocalServerId(const std::string & LocalServerIdFilename) {
    if (LocalServerIdFilename.empty()) {
        return;
    }
    auto File  = LocalServerIdFilename;
    auto FSOpt = std::ofstream(File, std::ios_base::binary | std::ios_base::out);
    if (!FSOpt) {
        cerr << "failed to dump file to LocalCacheFile" << endl;
        return;
    }
    FSOpt << LocalServerId << endl;
    return;
}
