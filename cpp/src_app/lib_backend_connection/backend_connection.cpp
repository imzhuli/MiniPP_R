#include "./backend_connection.hpp"

#include <pp_protocol/_backend/backend_challenge.hpp>
#include <pp_protocol/command.hpp>

bool xBackendConnection::Init(xel::xIoContext * ICP, const xel::xNetAddress & TargetAddress, const std::string & AppKey, const std::string & AppSecret) {
    State                                 = eState::Unspecified;
    this->ICP                             = ICP;
    this->TargetAddress                   = TargetAddress;
    this->State                           = eState::SetupReady;
    this->ConnectionStartTimestampMS      = 0;
    this->LastRequestKeepAliveTimestampMS = 0;
    this->LastKeepAliveTimestampMS        = 0;

    this->AppKey    = AppKey;
    this->AppSecret = AppSecret;

    return true;
}

void xBackendConnection::Clean() {
    if (State >= eState::Connecting && State <= eState::Closing) {
        Connection.Clean();
    }
    State = eState::Unspecified;
}

void xBackendConnection::Tick() {
    Ticker.Update();
    OnTick();
}

void xBackendConnection::Tick(uint64_t NowMS) {
    Ticker.Update(NowMS);
    OnTick();
}

void xBackendConnection::OnTick() {
    assert(State != eState::Unspecified);

    auto NowMS = Ticker();
    if (State == eState::Ready) {
        if ((NowMS - LastKeepAliveTimestampMS) >= MinKeepAliveTimeoutMS) {
            State = eState::Closing;
            goto CHECK_AND_RECONNECT;
        }
        if ((NowMS - LastRequestKeepAliveTimestampMS) >= MinRequestKeepAliveTimeoutMS) {
            X_DEBUG_PRINTF("PostRequestKeepAlive");
            Connection.PostRequestKeepAlive();
            LastRequestKeepAliveTimestampMS = NowMS;
        }
        return;
    }

    if (State == eState::Connecting || State == eState::Connected || State == eState::Challenging) {
        if ((NowMS - ConnectionStartTimestampMS) >= MinConnectionTimeoutMS) {
            State = eState::Closing;
        }
        // pass through
    }

CHECK_AND_RECONNECT:
    if (State == eState::Closing) {
        Connection.Clean();
        State = eState::SetupReady;
        // pass through
    }
    if (State == eState::SetupReady) {
        // reconnect:
        if (SignedDiff(NowMS, ConnectionStartTimestampMS) < MakeSigned(MinReconnectTimeoutMS)) {
            return;
        }
        X_DEBUG_PRINTF("Trying to connect");
        LastRequestKeepAliveTimestampMS = 0;
        LastKeepAliveTimestampMS        = 0;
        ConnectionStartTimestampMS      = NowMS;
        if (!Connection.Init(ICP, TargetAddress, this)) {
            return;
        }
        State = eState::Connecting;
    }
    return;
}

void xBackendConnection::OnConnected(xTcpConnection * TcpConnectionPtr) {
    X_DEBUG_PRINTF("");

    State                    = eState::Connected;
    auto challenge           = xBackendChallenge();
    challenge.AppKey         = AppKey;
    challenge.TimestampMS    = xel::GetTimestampMS();
    challenge.ChallengeValue = challenge.GenerateChallengeString(AppSecret);
    ubyte Buffer[xel::MaxPacketSize];
    auto  RSize = xel::WritePacket(Cmd_BackendChallenge, 0, Buffer, sizeof(Buffer), challenge);
    Connection.PostData(Buffer, RSize);

    X_DEBUG_PRINTF("Sending:\n%s", HexShow(Buffer, RSize).c_str());
    X_DEBUG_PRINTF("Header: %s", StrToHex(Buffer, PacketHeaderSize).c_str());
    X_DEBUG_PRINTF("Body: %s", StrToHex(Buffer + PacketHeaderSize, RSize - PacketHeaderSize).c_str());
    State = eState::Challenging;
}

void xBackendConnection::OnPeerClose(xTcpConnection * TcpConnectionPtr) {
    X_DEBUG_PRINTF("");

    State = eState::Closing;
}

size_t xBackendConnection::OnData(xTcpConnection * TcpConnectionPtr, ubyte * DataPtr, size_t DataSize) {
    assert(TcpConnectionPtr == &Connection);
    size_t RemainSize = DataSize;
    while (RemainSize >= PacketHeaderSize) {
        auto Header = xPacketHeader::Parse(DataPtr);
        if (!Header) { /* header error */
            return InvalidDataSize;
        }
        auto PacketSize = Header.PacketSize;  // make a copy, so Header can be reused
        if (RemainSize < PacketSize) {        // wait for data
            break;
        }
        if (Header.IsKeepAlive()) {
            X_DEBUG_PRINTF("KeepAlive");
            LastKeepAliveTimestampMS = Ticker();
        } else {
            auto PayloadPtr  = xPacket::GetPayloadPtr(DataPtr);
            auto PayloadSize = Header.GetPayloadSize();
            if (!OnPacket(Header, PayloadPtr, PayloadSize)) { /* packet error */
                X_DEBUG_PRINTF("Invalid packet");
                return InvalidDataSize;
            }
        }
        DataPtr    += PacketSize;
        RemainSize -= PacketSize;
    }
    return DataSize - RemainSize;
}

bool xBackendConnection::OnPacket(const xPacketHeader & Header, ubyte * PayloadPtr, size_t PayloadSize) {
    X_DEBUG_PRINTF("CommandId: %" PRIu32 ", RequestId:%" PRIx64 ": \n%s", Header.CommandId, Header.RequestId, HexShow(PayloadPtr, PayloadSize).c_str());
    if (State == eState::Ready) {
        return OnBackendPacket(Header, PayloadPtr, PayloadSize);
    }
    if (Header.CommandId != Cmd_BackendChallengeResp) {
        return false;
    }
    return OnCmdBackendChallengeResp(Header, PayloadPtr, PayloadSize);
}

bool xBackendConnection::OnCmdBackendChallengeResp(const xPacketHeader & Header, ubyte * PayloadPtr, size_t PayloadSize) {
    if (State != eState::Challenging) {
        return false;
    }

    auto R = xBackendChallengeResp();
    if (!R.Deserialize(PayloadPtr, PayloadSize)) {
        X_DEBUG_PRINTF("failed server challenge");
        return false;
    }

    State                           = eState::Ready;
    LastRequestKeepAliveTimestampMS = LastKeepAliveTimestampMS = Ticker();
    return OnConnectionReady();
}

bool xBackendConnection::OnConnectionReady() {
    X_DEBUG_PRINTF("Backend connection ready");
    return true;
}

bool xBackendConnection::PostData(const void * Data, size_t DataSize) {
    if (State != eState::Ready) {
        X_DEBUG_PRINTF("");
        return false;
    }
    Connection.PostData(Data, DataSize);
    return true;
}

bool xBackendConnection::PostMessage(xPacketCommandId CmdId, xPacketRequestId RequestId, xBinaryMessage & Message) {
    ubyte Buffer[MaxPacketSize];
    auto  PSize = WritePacket(CmdId, RequestId, Buffer, Message);
    if (!PSize) {
        X_DEBUG_PRINTF("");
        return false;
    }
    return PostData(Buffer, PSize);
}
