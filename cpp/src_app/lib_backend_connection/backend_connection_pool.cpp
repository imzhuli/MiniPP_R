#include "./backend_connection_pool.hpp"

#include <pp_protocol/_backend/backend_challenge.hpp>
#include <pp_protocol/command.hpp>

bool xBackendConnectionPool::Init(xIoContext * ICP, size_t MaxConnectionCount) {
    if (!xClientPool::Init(ICP, MaxConnectionCount)) {
        return false;
    }
    ContextList.resize(MaxConnectionCount);
    return true;
}

void xBackendConnectionPool::Clean() {
    xClientPool::Clean();
    Reset(ContextList);
}

uint64_t xBackendConnectionPool::AddServer(const xNetAddress & Address, const std::string & AppKey, const std::string & AppSecret) {
    auto Sid = xClientPool::AddServer(Address);
    if (!Sid) {
        return 0;
    }
    auto Index = Sid.GetIndex();
    assert(Index < ContextList.size());
    auto & Ctx = ContextList[Index];

    Ctx.AppKey    = AppKey;
    Ctx.AppSecret = AppSecret;
    return Sid;
}

void xBackendConnectionPool::RemoveServer(uint64_t ConnectionId) {
    auto Sid = xIndexId(ConnectionId);
    Reset(ContextList[Sid.GetIndex()]);
    xClientPool::RemoveServer(ConnectionId);
}

void xBackendConnectionPool::OnServerConnected(xClientConnection & CC) {
    auto   Sid = CC.GetConnectionId();
    auto   Idx = Sid.GetIndex();
    auto & Ctx = ContextList[Idx];

    auto challenge           = xBackendChallenge();
    challenge.AppKey         = Ctx.AppKey;
    challenge.TimestampMS    = xel::GetTimestampMS();
    challenge.ChallengeValue = challenge.GenerateChallengeString(Ctx.AppSecret);
    ubyte Buffer[xel::MaxPacketSize];
    auto  RSize = xel::WritePacket(Cmd_BackendChallenge, 0, Buffer, sizeof(Buffer), challenge);
    CC.PostData(Buffer, RSize);

    X_DEBUG_PRINTF("Sending:\n%s", HexShow(Buffer, RSize).c_str());
    X_DEBUG_PRINTF("Header: %s", StrToHex(Buffer, PacketHeaderSize).c_str());
    X_DEBUG_PRINTF("Body: %s", StrToHex(Buffer + PacketHeaderSize, RSize - PacketHeaderSize).c_str());
}

bool xBackendConnectionPool::OnServerPacket(xClientConnection & CC, const xPacketHeader & Header, ubyte * PayloadPtr, size_t PayloadSize) {
    if (Header.CommandId == Cmd_BackendChallengeResp) {
        return OnCmdBackendChallengeResp(CC, Header, PayloadPtr, PayloadSize);
    }

    auto   Sid = CC.GetConnectionId();
    auto   Idx = Sid.GetIndex();
    auto & Ctx = ContextList[Idx];
    X_DEBUG_PRINTF("ContextIndex = %" PRIu32 "", Idx);
    if (!Ctx.IsChallengeReady) {
        X_DEBUG_PRINTF("invalid data from challenge failed server");
        return false;
    }

    return OnBackendPacket(Header, PayloadPtr, PayloadSize);
}

void xBackendConnectionPool::OnServerClose(xClientConnection & CC) {
    auto   Sid = CC.GetConnectionId();
    auto   Idx = Sid.GetIndex();
    auto & Ctx = ContextList[Idx];
    Reset(Ctx);
}

bool xBackendConnectionPool::OnCmdBackendChallengeResp(xClientConnection & CC, const xPacketHeader & Header, ubyte * PayloadPtr, size_t PayloadSize) {
    auto   Sid = CC.GetConnectionId();
    auto   Idx = Sid.GetIndex();
    auto & Ctx = ContextList[Idx];

    if (Ctx.IsChallengeReady) {
        X_DEBUG_PRINTF("invalid challenge state");
        return false;
    }

    auto R = xBackendChallengeResp();
    if (!R.Deserialize(PayloadPtr, PayloadSize)) {
        X_DEBUG_PRINTF("failed server challenge");
        return false;
    }

    Ctx.IsChallengeReady = true;
    return true;
}
