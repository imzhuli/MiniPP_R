#include "./client_connection_manager.hpp"

#include "./_global.hpp"

#include <pp_protocol/command.hpp>
#include <pp_protocol/proxy_relay/connection.hpp>

static constexpr const uint64_t MAX_ACCOUNT_CHECK_TIMEOUT          = 3'000;
static constexpr const uint64_t MAX_CONNECTION_LIGNER_KILL_TIMEOUT = 10'000;

bool xPA_ClientConnectionManager::Init(xIoContext * ICP, const xNetAddress & BindAddress, size_t ConnectionPoolSize) {
    RuntimeAssert(TcpServer.Init(ICP, BindAddress, this));
    RuntimeAssert(ConnectionPool.Init(ConnectionPoolSize));

    return true;
}

void xPA_ClientConnectionManager::Clean() {
    ConnectionPool.Clean();
    TcpServer.Clean();
}

void xPA_ClientConnectionManager::Tick() {
    Ticker.Update();
    OnTick();
}

void xPA_ClientConnectionManager::Tick(uint64_t NowMS) {
    Ticker.Update(NowMS);
    OnTick();
}

void xPA_ClientConnectionManager::OnTick() {
    // remove timeout account connections
    while (auto PC = static_cast<xPA_ClientConnection *>(AccountTimeoutList.PopHead([KTP = Ticker() - MAX_ACCOUNT_CHECK_TIMEOUT](const xPA_AccountCheckNode & N) {
               return N.CheckAccountTimestampMS < KTP;
           }))) {
        CleanupConnection(*PC);
    }

    // remove linger killed connections
    while (auto PC =
               static_cast<xPA_ClientConnection *>(LingerKillConnectionList.PopHead([KTP = Ticker() - MAX_CONNECTION_LIGNER_KILL_TIMEOUT](const xPA_KillClientConnectionNode & N) {
                   return N.LingerKillTimestamp < KTP;
               }))) {
        CleanupConnection(*PC);
    }

    // remove killed connections
    while (auto PC = static_cast<xPA_ClientConnection *>(KillConnectionList.PopHead())) {
        CleanupConnection(*PC);
    }
}

void xPA_ClientConnectionManager::OnNewConnection(xTcpServer * TcpServerPtr, xSocket && NativeHandle) {
    auto CID = ConnectionPool.Acquire();
    if (!CID) {
        XelCloseSocket(NativeHandle);
        return;
    }
    auto & C = ConnectionPool[CID];
    if (!C.Init(TcpServerPtr->GetIoContextPtr(), std::move(NativeHandle), this)) {
        X_DEBUG_PRINTF("Failed to create new connection object");
        ConnectionPool.Release(CID);
        return;
    }
    C.ConnectionId            = CID;
    C.CheckAccountTimestampMS = Ticker();
    AccountTimeoutList.AddTail(C);
}

void xPA_ClientConnectionManager::CleanupConnection(xPA_ClientConnection & Conn) {
    assert(&Conn == ConnectionPool.CheckAndGet(Conn.ConnectionId));
    // TODO: 检查并清理与中转服之间的状态

    Conn.Clean();
    ConnectionPool.Release(Conn.ConnectionId);
}

size_t xPA_ClientConnectionManager::OnData(xTcpConnection * TcpConnectionPtr, ubyte * DataPtr, size_t DataSize) {
    // X_DEBUG_PRINTF("\n%s", HexShow(DataPtr, DataSize).c_str());
    auto CP = (xPA_ClientConnection *)TcpConnectionPtr;
    switch (CP->Phase) {
        case xPA_ClientConnection::eUnknown:
            return OnChallenge(CP, DataPtr, DataSize);

        case xPA_ClientConnection::eS5WaitForAuthInfo:
            return OnS5ClientAuth(CP, DataPtr, DataSize);
        case xPA_ClientConnection::eS5WaitForConnectionRequest:
            return OnS5ConnectionRequest(CP, DataPtr, DataSize);
        case xPA_ClientConnection::eS5ConnectionReady:
            return OnS5UploadData(CP, DataPtr, DataSize);

        case xPA_ClientConnection::eHttpRawChallenge:
            return OnHttpRawChallenge(CP, DataPtr, DataSize);
        case xPA_ClientConnection::eHttpRawReady:
            return OnHttpRawUploadData(CP, DataPtr, DataSize);

        case xPA_ClientConnection::eHttpNormalChallenge:
            return OnHttpNormalChallenge(CP, DataPtr, DataSize);
        case xPA_ClientConnection::eHttpNormalReady:
            return OnHttpNormalUploadData(CP, DataPtr, DataSize);

        default:
            X_DEBUG_PRINTF("Unknown processed phase, closing connection: %u", (unsigned)CP->Phase);
            break;
    }
    Kill(*CP);
    return 0;
}

void xPA_ClientConnectionManager::OnPeerClose(xTcpConnection * TcpConnectionPtr) {
    auto CCP = static_cast<xPA_ClientConnection *>(TcpConnectionPtr);
    if (CCP->Phase == xPA_ClientConnection::eS5ConnectionReady) {
        // TODO
        X_DEBUG_PRINTF("TODO: Tell Relay Server To Close Relay Side Connection");
        CCP->Phase = xPA_ClientConnection::eS5WaitForConnectionClosed;
    }
    Kill(*static_cast<xPA_ClientConnection *>(TcpConnectionPtr));
}

size_t xPA_ClientConnectionManager::OnChallenge(xPA_ClientConnection * ConnectionPtr, const void * DataPtr, size_t DataSize) {
    // minimal challenge header:
    if (DataSize < 3) {
        return 0;
    }
    if (((const ubyte *)DataPtr)[0] == 0x05) {  // version : S5
        return OnS5Challenge(ConnectionPtr, DataPtr, DataSize);
    }

    auto HttpProcessed = OnHttpChallenge(ConnectionPtr, DataPtr, DataSize);
    if (HttpProcessed != InvalidDataSize) {
        auto RemainDataPtr = static_cast<const ubyte *>(DataPtr) + HttpProcessed;
        auto Remained      = DataSize - HttpProcessed;
        Touch(RemainDataPtr, Remained);
    }
    return HttpProcessed;
}

size_t xPA_ClientConnectionManager::OnS5Challenge(xPA_ClientConnection * ConnectionPtr, const void * DataPtr, size_t DataSize) {
    X_DEBUG_PRINTF("");

    auto R = xStreamReader(DataPtr);
    R.Skip(1);  // skip type check bytes

    size_t NM = R.R1();  // number of methods
    if (!NM) {
        Kill(*ConnectionPtr);
        return 0;
    }
    size_t HeaderSize = 2 + NM;
    if (DataSize < HeaderSize) {
        return 0;
    }
    bool UserPassSupport = false;
    bool NoAuthSupport   = false;
    Touch(NoAuthSupport);
    for (size_t i = 0; i < NM; ++i) {
        uint8_t Method = R.R1();
        if (Method == 0x02) {
            UserPassSupport = true;
            continue;
        }
        // if (Method == 0x00) {
        //     NoAuthSupport = true;
        //     continue;
        // }
    }
    if (UserPassSupport) {
        ubyte Socks5Auth[2] = { 0x05, 0x02 };
        ConnectionPtr->PostData(Socks5Auth, sizeof(Socks5Auth));
    } else if (NoAuthSupport) {
        ubyte Socks5Auth[2] = { 0x05, 0x00 };
        ConnectionPtr->PostData(Socks5Auth, sizeof(Socks5Auth));
    } else {
        X_DEBUG_PRINTF("Unsupported auth method");
        ubyte Socks5Auth[2] = { 0x05, 0xFF };
        ConnectionPtr->PostData(Socks5Auth, sizeof(Socks5Auth));
        Kill(*ConnectionPtr);
    }

    ConnectionPtr->Phase = xPA_ClientConnection::eS5WaitForAuthInfo;
    X_DEBUG_PRINTF("eS5WaitForAuthInfo");
    return HeaderSize;
}

size_t xPA_ClientConnectionManager::OnHttpChallenge(xPA_ClientConnection * ConnectionPtr, const void * DataPtr, size_t DataSize) {
    X_DEBUG_PRINTF("");

    std::string_view HostnameView;
    uint16_t         Port = 0;

    std::string_view DataView   = { (const char *)DataPtr, DataSize };
    auto             LineStart  = DataView.data();
    auto             LineLength = DataView.find("\r\n");
    if (LineLength == 0) {
        X_DEBUG_PRINTF("Invalid Http challenge, new line at beginning");
        Kill(*ConnectionPtr);
        return 0;
    } else if (LineLength == DataView.npos) {
        return 0;
    }
    if (0 == strncasecmp("CONNECT ", LineStart, 8)) {
        auto LineEnd = LineStart + LineLength;
        LineStart   += 8;  // skip CONNECT & space
        for (auto Curr = LineStart; Curr < LineEnd; ++Curr) {
            char C = *Curr;
            if (C == ':') {
                HostnameView = { LineStart, (size_t)(Curr - LineStart) };
                Port         = atoi(++Curr);
                break;
            } else if (C == ' ') {
                HostnameView = { LineStart, (size_t)(Curr - LineStart) };
                Port         = 80;
                break;
            }
        }
        if (!HostnameView.length()) {
            X_DEBUG_PRINTF("Invalid HttpProxy Target");
            Kill(*ConnectionPtr);
            return 0;
        }
        ConnectionPtr->Http.TargetHost = std::string(HostnameView);
        ConnectionPtr->Http.TargetPort = Port;
        ConnectionPtr->Phase           = xPA_ClientConnection::eHttpRawChallenge;

        X_DEBUG_PRINTF("RawChallenge: %s:%u", ConnectionPtr->Http.TargetHost.c_str(), (unsigned)ConnectionPtr->Http.TargetPort);
        return LineLength + 2;
    }

    auto LineView      = std::string_view{ LineStart, LineLength };
    auto URLStartIndex = LineView.find(' ');
    if (URLStartIndex == LineView.npos) {
        X_DEBUG_PRINTF("Invalid Http Target Line");
        Kill(*ConnectionPtr);
        return 0;
    }
    ++URLStartIndex;
    ConnectionPtr->Http.RebuiltHttpHeader.append(LineStart, URLStartIndex);

    auto URLEndIndex = LineView.find(' ', URLStartIndex);
    if (URLEndIndex == LineView.npos) {
        X_DEBUG_PRINTF("Invalid Http Target Line");
        Kill(*ConnectionPtr);
        return 0;
    }
    auto URLStart = LineView.data() + URLStartIndex;
    if (0 == strncasecmp(URLStart, "http://", 7)) {
        URLStart      += 7;
        URLStartIndex += 7;
    } else if (0 == strncasecmp(URLStart, "https://", 8)) {
        URLStart      += 8;
        URLStartIndex += 7;
    }

    auto PathStartIndex = LineView.find('/', URLStartIndex);
    if (PathStartIndex == LineView.npos) {
        X_DEBUG_PRINTF("Invalid Http Target Line (No Path)");
        Kill(*ConnectionPtr);
        return 0;
    }
    // get host:port
    for (auto Curr = URLStart; Curr <= LineStart + PathStartIndex; ++Curr) {
        char C = *Curr;
        if (C == ':') {
            HostnameView = { URLStart, (size_t)(Curr - URLStart) };
            Port         = atoi(++Curr);
            break;
        } else if (C == '/') {
            HostnameView = { URLStart, (size_t)(Curr - URLStart) };
            Port         = 80;
            break;
        }
    }

    ConnectionPtr->Http.TargetHost = std::string(HostnameView);
    ConnectionPtr->Http.TargetPort = Port;
    ConnectionPtr->Http.RebuiltHttpHeader.append(LineStart + PathStartIndex, LineLength - PathStartIndex + 2);
    ConnectionPtr->Phase = xPA_ClientConnection::eHttpNormalChallenge;
    X_DEBUG_PRINTF("NormalChallenge to %s:%u", ConnectionPtr->Http.TargetHost.c_str(), (unsigned)ConnectionPtr->Http.TargetPort);

    return LineLength + 2;
}

size_t xPA_ClientConnectionManager::OnS5ClientAuth(xPA_ClientConnection * ConnectionPtr, void * DataPtr, size_t DataSize) {
    X_DEBUG_PRINTF("");
    if (DataSize < 3) {
        return 0;
    }

    xStreamReader R(DataPtr);
    auto          Ver = R.R1();
    auto          NP  = ConnectionPtr->GetRemoteAddress().IpToString();
    if (Ver == 0x01) {  // only version for user pass
        size_t NameLen = R.R1();
        if (DataSize < 3 + NameLen) {
            return 0;
        }
        // auto   NameView = std::string_view((const char *)R(), NameLen);
        char * KeyStart = (char *)DataPtr + R.Offset();
        R.Skip(NameLen);

        size_t PassLen = R.R1();
        if (DataSize < 3 + NameLen + PassLen) {
            X_DEBUG_PRINTF("Wait for auth data");
            return 0;
        }
        // auto PassView                     = std::string_view((const char *)R(), PassLen);
        ((char *)DataPtr)[R.Offset() - 1] = ':';  // make quick user/pass key
        R.Skip(PassLen);
        if (NameLen || PassLen) {
            NP = std::string{ KeyStart, NameLen + PassLen + 1 };
        } else {
            X_DEBUG_PRINTF("Empty user:pass found, using IP");
        }
    } else if (Ver == 0x00) {  // No Auth
        X_DEBUG_PRINTF("Empty S5 AuthInfo, using IP");
    } else {
        X_DEBUG_PRINTF("AuthMethod: %u", (unsigned)Ver);
        return InvalidDataSize;
    }
    X_DEBUG_PRINTF("AuthInfo AuthType=%u: NP=%s", (unsigned)Ver, NP.c_str());

    GlobalAuthCacheManager.RequestAuth(ConnectionPtr->ConnectionId, NP);
    ConnectionPtr->Phase = xPA_ClientConnection::eS5WaitForAccountExchange;
    return R.Offset();
}

void xPA_ClientConnectionManager::OnS5ClientAuthFinished(xPA_ClientConnection * CCP, const xPA_AuthResult * ARP) {
    if (!ARP) {
        X_DEBUG_PRINTF("Invalid Auth Result, check connection");
        CCP->PostData("\x01\x01", 2);  // S5 auth failure
        LingerKill(*CCP);
        return;
    }

    AccountTimeoutList.Remove(*CCP);
    KeepAlive(*CCP);

    // Select device:
    auto R               = xPA_DeviceRequest();
    R.ClientConnectionId = CCP->ConnectionId;
    R.CountryId          = ARP->CountryId;
    R.StateId            = ARP->StateId;
    R.CityId             = ARP->CityId;

    GlobalDeviceSelectorManager.PostDeviceRequest(R);
    CCP->Phase = xPA_ClientConnection::eS5WaitForDeviceSelection;
}

size_t xPA_ClientConnectionManager::OnS5ConnectionRequest(xPA_ClientConnection * CCP, void * DataPtr, size_t DataSize) {

    if (DataSize < 10) {
        return 0;
    }
    if (DataSize >= 6 + 256) {
        X_DEBUG_PRINTF("Very big connection request, which is obviously wrong");
        Kill(*CCP);
        return 0;
    }
    xStreamReader R(DataPtr);
    uint8_t       Version   = R.R();
    uint8_t       Operation = R.R();
    uint8_t       Reserved  = R.R();
    uint8_t       AddrType  = R.R();
    if (Version != 0x05 || Reserved != 0x00) {
        X_DEBUG_PRINTF("Non Socks5 connection request");
        Kill(*CCP);
        return 0;
    }
    xNetAddress Address;
    char        DomainName[256]  = {};
    size_t      DomainNameLength = 0;
    if (AddrType == 0x01) {  // ipv4
        Address.Type = xNetAddress::IPV4;
        R.R(Address.SA4, 4);
        Address.Port = R.R2();
    } else if (AddrType == 0x03) {  // domain
        DomainNameLength = R.R();
        if (DataSize < 4 + 1 + DomainNameLength + 2) {
            return 0;
        }
        R.R(DomainName, DomainNameLength);
        DomainName[DomainNameLength] = '\0';
        Address.Port                 = R.R2();
    } else if (AddrType == 0x04) {  // ipv6
        if (DataSize < 4 + 16 + 2) {
            return 0;
        }
        Address.Type = xNetAddress::IPV6;
        R.R(Address.SA6, 16);
        Address.Port = R.R2();
    } else {
        X_DEBUG_PRINTF("Invalid connection request");
        Kill(*CCP);
        return 0;
    }
    size_t AddressLength = R.Offset() - 3;
    if (Operation != 0x01 && AddrType != 0x03 && AddrType != 0x04) {
        X_DEBUG_PRINTF("Operation other than tcp ipv4/ipv6/domain connection");
        ubyte         Buffer[512];
        xStreamWriter W(Buffer);
        W.W(0x05);
        W.W(0x01);
        W.W(0x00);
        W.W((ubyte *)DataPtr + 3, AddressLength);
        CCP->PostData(Buffer, W.Offset());
        LingerKill(*CCP);
        return 0;
    }

    X_DEBUG_PRINTF("AddressType: %u, Domain(if exists): %s", (unsigned)AddrType, DomainName);
    if (AddrType == 0x01 || AddrType == 0x04) {
        if (!Address || !Address.Port) {
            X_DEBUG_PRINTF("Invalid address type");
        }
    }
    OnOpenRemoteConnection(CCP, Address, std::string_view(DomainName, DomainNameLength));
    return DataSize;
}

size_t xPA_ClientConnectionManager::OnS5UploadData(xPA_ClientConnection * CCP, void * VoidDP, size_t DataSize) {
    auto RCP = GlobalTestRCM.GetConnectionById(CCP->RelayConnectionId);
    if (!RCP) {
        X_DEBUG_PRINTF("Relay connection lost: RelayConnectionId:%" PRIx64 "", CCP->RelayConnectionId);
        return InvalidDataSize;
    }

    auto DataPtr                      = (const char *)VoidDP;
    auto RemainSize                   = DataSize;
    auto PushRequest                  = xPR_PushData{};
    PushRequest.RelaySideConnectionId = CCP->RelaySideConnectionId;
    PushRequest.ProxySideConnectionId = CCP->ConnectionId;

    while (RemainSize) {
        size_t PostSize         = std::min(RemainSize, xPR_PushData::MAX_PAYLOAD_SIZE);
        PushRequest.PayloadView = { (const char *)DataPtr, PostSize };

        X_DEBUG_PRINTF("%s", HexShow(DataPtr, PostSize).c_str());
        RCP->PostMessage(Cmd_PA_RL_PostData, 0, PushRequest);

        DataPtr    += PostSize;
        RemainSize -= PostSize;
    }

    TryUpdateDataTransferSize(CCP, DataSize, 0);
    KeepAlive(*CCP);
    return DataSize;
}

size_t xPA_ClientConnectionManager::OnHttpRawChallenge(xPA_ClientConnection * ConnectionPtr, const void * DataPtr, size_t DataSize) {
    X_DEBUG_PRINTF("");

    auto   HeaderView   = std::string_view{ (const char *)DataPtr, DataSize };
    auto   AuthNamePass = std::string();
    size_t ConsumedSize = 0;
    while (true) {
        auto LineEndIndex = HeaderView.find("\r\n");
        if (LineEndIndex == 0) {
            if (AuthNamePass.empty()) {
                X_DEBUG_PRINTF("Invalid Proxy-Authorization: Not Found!");
                ConnectionPtr->PostData("HTTP/1.1 407 Proxy Authentication Required\r\nProxy-Authenticate: Basic\r\nConnection: close\r\n\r\n", 92);
                LingerKill(*ConnectionPtr);
                return 0;
            }
            ConsumedSize += 2;

            X_DEBUG_PRINTF("RequestAuth: %s", AuthNamePass.c_str());
            GlobalAuthCacheManager.RequestAuth(ConnectionPtr->ConnectionId, AuthNamePass);

            ConnectionPtr->SuspendReading();
            ConnectionPtr->Phase = xPA_ClientConnection::eHttpRawWaitForAccountExchange;
            return ConsumedSize;
        }
        if (LineEndIndex == HeaderView.npos) {
            return ConsumedSize;
        };
        auto LineLength = LineEndIndex + 2;
        ConsumedSize   += LineLength;

        auto LineStart = HeaderView.data();
        if (LineEndIndex > 21 && 0 == strncasecmp(LineStart, "Proxy-Authorization: ", 21)) {
            auto AuthStart  = LineStart + 21;
            auto AuthLength = LineEndIndex - 21;
            if (AuthLength < 6 && 0 != strncasecmp(AuthStart, "Basic ", 6)) {
                X_DEBUG_PRINTF("Invalid Proxy-Authorization Request");
                Kill(*ConnectionPtr);
                return 0;
            }
            auto Base64Start = AuthStart + 6;
            auto Base64Size  = AuthLength - 6;
            AuthNamePass     = Base64Decode(Base64Start, Base64Size);
        }
        HeaderView = HeaderView.substr(LineLength);
    }
    X_DEBUG_PRINTF("BUG: Impossible loop exit");
    return InvalidDataSize;
}

void xPA_ClientConnectionManager::OnHttpRawAuthFinished(xPA_ClientConnection * CCP, const xPA_AuthResult * ARP) {
    if (!ARP) {
        X_DEBUG_PRINTF("Invalid Proxy-Authorization: Not Found!");
        CCP->PostData("HTTP/1.1 407 Proxy Authentication Required\r\nProxy-Authenticate: Basic\r\nConnection: close\r\n\r\n", 92);
        Kill(*CCP);
        return;
    }

    AccountTimeoutList.Remove(*CCP);
    KeepAlive(*CCP);

    X_DEBUG_PRINTF("SelectedRegionIfno: %u/%u/%u", (unsigned)ARP->CountryId, (unsigned)ARP->StateId, (unsigned)ARP->CityId);
    // Select device:
    auto R               = xPA_DeviceRequest();
    R.ClientConnectionId = CCP->ConnectionId;
    R.CountryId          = ARP->CountryId;
    R.StateId            = ARP->StateId;
    R.CityId             = ARP->CityId;

    GlobalDeviceSelectorManager.PostDeviceRequest(R);
    CCP->Phase = xPA_ClientConnection::eHttpRawWaitForDeviceSelection;
    return;
}

size_t xPA_ClientConnectionManager::OnHttpRawUploadData(xPA_ClientConnection * CCP, void * VoidDP, size_t DataSize) {
    auto RCP = GlobalTestRCM.GetConnectionById(CCP->RelayConnectionId);
    if (!RCP) {
        X_DEBUG_PRINTF("Relay connection lost: RelayConnectionId:%" PRIx64 "", CCP->RelayConnectionId);
        return InvalidDataSize;
    }

    auto DataPtr                      = (const char *)VoidDP;
    auto RemainSize                   = DataSize;
    auto PushRequest                  = xPR_PushData{};
    PushRequest.RelaySideConnectionId = CCP->RelaySideConnectionId;
    PushRequest.ProxySideConnectionId = CCP->ConnectionId;

    while (RemainSize) {
        size_t PostSize         = std::min(RemainSize, xPR_PushData::MAX_PAYLOAD_SIZE);
        PushRequest.PayloadView = { (const char *)DataPtr, PostSize };

        X_DEBUG_PRINTF("\n%s", HexShow(DataPtr, PostSize).c_str());
        RCP->PostMessage(Cmd_PA_RL_PostData, 0, PushRequest);

        DataPtr    += PostSize;
        RemainSize -= PostSize;
    }

    TryUpdateDataTransferSize(CCP, DataSize, 0);
    KeepAlive(*CCP);
    return DataSize;
}

size_t xPA_ClientConnectionManager::OnHttpNormalChallenge(xPA_ClientConnection * ConnectionPtr, const void * DataPtr, size_t DataSize) {
    auto   ConsumedSize = 0;
    auto   DataView     = std::string_view{ (const char *)DataPtr, DataSize };
    auto & Http         = ConnectionPtr->Http;
    auto & LineRef      = Http.HttpHeaderLine;
    while (true) {
        // fill line:
        auto LineEndIndex = DataView.find("\r\n");
        if (LineEndIndex == DataView.npos) {
            LineRef.append(DataView.data(), DataView.length());
            return DataSize;
        }
        if (LineEndIndex == 0) {
            if (LineRef.empty()) {
                ConsumedSize += 2;
                break;  // process whole header
            }
        }
        LineRef.append(DataView.data(), LineEndIndex);
        ConsumedSize += LineEndIndex + 2;
        DataView      = DataView.substr(LineEndIndex + 2);

        // process line
        if (LineRef.length() > 18 && 0 == strncasecmp(LineRef.data(), "Proxy-Connection: ", 18)) {
            Pass();
        } else if (LineRef.length() > 12 && 0 == strncasecmp(LineRef.data(), "Connection: ", 12)) {
            Http.RebuiltHttpHeader.append("Connection: close\r\n", 19);
        } else if (LineRef.length() > 21 && 0 == strncasecmp(LineRef.data(), "Proxy-Authorization: ", 21)) {
            auto AuthStart  = LineRef.data() + 21;
            auto AuthLength = LineEndIndex - 21;
            if (AuthLength < 6 && 0 != strncasecmp(AuthStart, "Basic ", 6)) {
                X_DEBUG_PRINTF("Invalid Proxy-Authorization Request");
                Kill(*ConnectionPtr);
                return 0;
            }
            auto Base64Start  = AuthStart + 6;
            auto Base64Size   = AuthLength - 6;
            Http.AuthNamePass = Base64Decode(Base64Start, Base64Size);
            X_DEBUG_PRINTF("HttpAuth: %s", Http.AuthNamePass.c_str());
        } else if (LineRef.length() > 16 && 0 == strncasecmp(LineRef.data(), "Content-Length: ", 16)) {
            Http.ContentLengthLeft = atoll(LineRef.data() + 16);
            Http.RebuiltHttpHeader.append(LineRef);
            Http.RebuiltHttpHeader.append("\r\n", 2);
        } else {
            Http.RebuiltHttpHeader.append(LineRef);
            Http.RebuiltHttpHeader.append("\r\n", 2);
        }

        // clear line:
        LineRef.clear();
    }
    Http.RebuiltHttpHeader.append("\r\n", 2);
    X_DEBUG_PRINTF("RebuiltHttpHeader: ConsumedSize=%d\n%s", ConsumedSize, Http.RebuiltHttpHeader.c_str());

    // Header Done
    if (Http.AuthNamePass.empty()) {
        X_DEBUG_PRINTF("Missing auth info");
        ConnectionPtr->PostData("HTTP/1.1 407 Proxy Authentication Required\r\nProxy-Authenticate: Basic\r\nConnection: close\r\n\r\n", 92);
        LingerKill(*ConnectionPtr);
        return ConsumedSize;
    }
    GlobalAuthCacheManager.RequestAuth(ConnectionPtr->ConnectionId, Http.AuthNamePass);
    ConnectionPtr->SuspendReading();
    ConnectionPtr->Phase = xPA_ClientConnection::eHttpNormalWaitForAccountExchange;
    return ConsumedSize;
}

void xPA_ClientConnectionManager::OnHttpNormalAuthFinished(xPA_ClientConnection * CCP, const xPA_AuthResult * ARP) {
    if (!ARP) {
        X_DEBUG_PRINTF("Invalid Proxy-Authorization: Not Found!");
        CCP->PostData("HTTP/1.1 407 Proxy Authentication Required\r\nProxy-Authenticate: Basic\r\nConnection: close\r\n\r\n", 92);
        Kill(*CCP);
        return;
    }

    AccountTimeoutList.Remove(*CCP);
    KeepAlive(*CCP);

    X_DEBUG_PRINTF("SelectedRegionIfno: %u/%u/%u", (unsigned)ARP->CountryId, (unsigned)ARP->StateId, (unsigned)ARP->CityId);
    // Select device:
    auto R               = xPA_DeviceRequest();
    R.ClientConnectionId = CCP->ConnectionId;
    R.CountryId          = ARP->CountryId;
    R.StateId            = ARP->StateId;
    R.CityId             = ARP->CityId;

    GlobalDeviceSelectorManager.PostDeviceRequest(R);
    CCP->Phase = xPA_ClientConnection::eHttpNormalWaitForDeviceSelection;
    return;
}

size_t xPA_ClientConnectionManager::OnHttpNormalUploadData(xPA_ClientConnection * CCP, void * VoidDP, size_t DataSize) {
    auto RCP = GlobalTestRCM.GetConnectionById(CCP->RelayConnectionId);
    if (!RCP) {
        X_DEBUG_PRINTF("Relay connection lost: RelayConnectionId:%" PRIx64 "", CCP->RelayConnectionId);
        return InvalidDataSize;
    }

    auto DataPtr                      = (const char *)VoidDP;
    auto RemainSize                   = DataSize;
    auto PushRequest                  = xPR_PushData{};
    PushRequest.RelaySideConnectionId = CCP->RelaySideConnectionId;
    PushRequest.ProxySideConnectionId = CCP->ConnectionId;

    while (RemainSize) {
        size_t PostSize         = std::min(RemainSize, xPR_PushData::MAX_PAYLOAD_SIZE);
        PushRequest.PayloadView = { (const char *)DataPtr, PostSize };

        X_DEBUG_PRINTF("\n%s", HexShow(DataPtr, PostSize).c_str());
        RCP->PostMessage(Cmd_PA_RL_PostData, 0, PushRequest);

        DataPtr    += PostSize;
        RemainSize -= PostSize;
    }

    TryUpdateDataTransferSize(CCP, DataSize, 0);
    KeepAlive(*CCP);
    return DataSize;
}

void xPA_ClientConnectionManager::OnAuthResult(uint64_t SourceClientConnectionId, const xPA_AuthResult * PR) {
    auto PC = GetConnectionById(SourceClientConnectionId);
    if (!PC) {
        X_DEBUG_PRINTF("Connection lost");
        return;
    }
    X_DEBUG_PRINTF("ConnectionId: %" PRIx64 "", SourceClientConnectionId);

    switch (PC->Phase) {
        case xPA_ClientConnection::eS5WaitForAccountExchange:
            OnS5ClientAuthFinished(PC, PR);
            return;

        case xPA_ClientConnection::eHttpRawWaitForAccountExchange:
            OnHttpRawAuthFinished(PC, PR);
            return;

        case xPA_ClientConnection::eHttpNormalWaitForAccountExchange:
            OnHttpNormalAuthFinished(PC, PR);
            return;

        default: {
            X_DEBUG_PRINTF("Invalid connection state");
            Kill(*PC);
            return;
        }
    }
}

void xPA_ClientConnectionManager::OnDeviceSelected(const xPA_DeviceRequestResp & Result) {
    auto CCP = GetConnectionById(Result.ClientConnectionId);
    if (!CCP) {
        X_DEBUG_PRINTF("ClientConnection Not Found: id=%" PRIx64 "", Result.ClientConnectionId);
        return;
    }

    switch (CCP->Phase) {
        case xPA_ClientConnection::eS5WaitForDeviceSelection: {
            X_DEBUG_PRINTF("try using device : %" PRIx64 ", -> %" PRIx64 "", Result.RelayServerRuntimeId, Result.DeviceRelaySideId);

            if (!Result.DeviceRelaySideId) {
                X_DEBUG_PRINTF("invalid remote device");
                CCP->PostData("\x01\x01", 2);  // S5 auth failure
                Kill(*CCP);
                return;
            }

            CCP->RelayConnectionId = Result.RelayServerRuntimeId;
            CCP->RelaySideDeviceId = Result.DeviceRelaySideId;

            // auth done and device selected
            CCP->PostData("\x01\x00", 2);  // S5
            CCP->Phase = xPA_ClientConnection::eS5WaitForConnectionRequest;
            return;
        }

        case xPA_ClientConnection::eHttpRawWaitForDeviceSelection: {

            CCP->RelayConnectionId = Result.RelayServerRuntimeId;
            CCP->RelaySideDeviceId = Result.DeviceRelaySideId;

            auto Address = xNetAddress::Parse(CCP->Http.TargetHost);
            Address.Port = CCP->Http.TargetPort;
            OnOpenRemoteConnection(CCP, Address, CCP->Http.TargetHost);
            return;
        }

        case xPA_ClientConnection::eHttpNormalWaitForDeviceSelection: {

            CCP->RelayConnectionId = Result.RelayServerRuntimeId;
            CCP->RelaySideDeviceId = Result.DeviceRelaySideId;

            auto Address = xNetAddress::Parse(CCP->Http.TargetHost);
            Address.Port = CCP->Http.TargetPort;
            OnOpenRemoteConnection(CCP, Address, CCP->Http.TargetHost);
            return;
        }

        default: {
            X_DEBUG_PRINTF("Invalid connection state");
            Kill(*CCP);
            return;
        }
    }
}

void xPA_ClientConnectionManager::OnOpenRemoteConnection(xPA_ClientConnection * CCP, const xNetAddress & Address, const std::string_view HostnameView) {
    auto RCP = GlobalTestRCM.GetConnectionById(CCP->RelayConnectionId);
    if (!RCP) {
        X_DEBUG_PRINTF("Relay connection not found");
        return;
    }

    X_DEBUG_PRINTF("Found RelayServer: @%s", RCP->GetTargetAddress().ToString().c_str());

    // build relay context
    auto Cmd = Cmd_PA_RL_CreateConnection;
    auto Req = xPR_CreateConnection();

    Req.RelaySideDeviceId     = CCP->RelaySideDeviceId;
    Req.ProxySideConnectionId = CCP->ConnectionId;
    Req.TargetAddress         = Address;
    Req.HostnameView          = HostnameView;
    Req.HostnamePort          = Address.Port;

    X_DEBUG_PRINTF("Try Open Connection: %s, %s:%u", Address.ToString().c_str(), std::string(Req.HostnameView).c_str(), (unsigned)Req.HostnamePort);

    ///
    RCP->PostMessage(Cmd, 0, Req);
    if (CCP->Phase == xPA_ClientConnection::eS5WaitForConnectionRequest) {
        CCP->Phase = xPA_ClientConnection::eS5WaitForConnectionEstablish;
    } else if (CCP->Phase == xPA_ClientConnection::eHttpRawWaitForDeviceSelection) {
        CCP->Phase = xPA_ClientConnection::eHttpRawWaitForConnectionEstablish;
    } else if (CCP->Phase == xPA_ClientConnection::eHttpNormalWaitForDeviceSelection) {
        CCP->Phase = xPA_ClientConnection::eHttpNormalWaitForConnectionEstablish;
    }
}

void xPA_ClientConnectionManager::OnRelaySideConnectionStateChange(const ubyte * PayloadPtr, size_t PayloadSize) {
    auto N = xPR_ConnectionStateNotify();
    if (!N.Deserialize(PayloadPtr, PayloadSize)) {
        X_DEBUG_PRINTF("Invalid protocol");
        return;
    }
    auto CCP = GetConnectionById(N.ProxySideConnectionId);
    if (!CCP) {
        X_DEBUG_PRINTF("ClientConnection not found: %" PRIx64 "", N.ProxySideConnectionId);
        return;
    }
    switch (CCP->Phase) {
        case xPA_ClientConnection::eS5WaitForConnectionEstablish: {
            RuntimeAssert(N.NewState == xPR_ConnectionStateNotify::STATE_ESTABLISHED);
            static constexpr const ubyte Reply[] = {
                '\x05', '\x00', '\x00',          // ok
                '\x01',                          // ipv4
                '\x00', '\x00', '\x00', '\x00',  // ip: 0.0.0.0
                '\x00', '\x00',                  // port 0:
            };
            CCP->PostData(Reply, sizeof(Reply));

            CCP->RelaySideConnectionId = N.RelaySideConnectionId;
            CCP->Phase                 = xPA_ClientConnection::eS5ConnectionReady;
            break;
        }
        case xPA_ClientConnection::eS5ConnectionReady: {
            switch (N.NewState) {
                case xPR_ConnectionStateNotify::STATE_CLOSED: {
                    CCP->Phase = xPA_ClientConnection::eS5WaitForConnectionClosed;
                    if (CCP->HasPendingWrites()) {
                        LingerKill(*CCP);
                    } else {
                        Kill(*CCP);
                    }
                    break;
                }
                case xPR_ConnectionStateNotify::STATE_UPDATE_TRANSFER: {
                    X_DEBUG_PRINTF("Unprocessed udpate");
                    break;
                }

                default:
                    X_DEBUG_PRINTF("Invalid data state");
                    break;
            }
            break;
        }
        case xPA_ClientConnection::xPA_ClientConnection::eHttpRawWaitForConnectionEstablish: {
            RuntimeAssert(N.NewState == xPR_ConnectionStateNotify::STATE_ESTABLISHED);
            CCP->ResumeReading();
            CCP->PostData("HTTP/1.1 200 Connection established\r\nProxy-agent: proxy / 1.0\r\n\r\n", 65);

            CCP->RelaySideConnectionId = N.RelaySideConnectionId;
            CCP->Phase                 = xPA_ClientConnection::eHttpRawReady;
            break;
        }
        case xPA_ClientConnection::xPA_ClientConnection::eHttpNormalWaitForConnectionEstablish: {
            RuntimeAssert(N.NewState == xPR_ConnectionStateNotify::STATE_ESTABLISHED);
            CCP->ResumeReading();
            CCP->RelaySideConnectionId = N.RelaySideConnectionId;
            CCP->Phase                 = xPA_ClientConnection::eHttpNormalReady;

            OnHttpNormalUploadData(CCP, CCP->Http.RebuiltHttpHeader.data(), CCP->Http.RebuiltHttpHeader.size());
            break;
        }

        default:
            X_DEBUG_PRINTF("Invalid connection state, Ignored: %u", (unsigned)CCP->Phase);
            break;
    }

    X_DEBUG_PRINTF("%s", N.ToString().c_str());
}

void xPA_ClientConnectionManager::OnDestroyConnection(const ubyte * PayloadPtr, size_t PayloadSize) {
    X_DEBUG_PRINTF("%s", HexShow(PayloadPtr, PayloadSize).c_str());

    auto N = xPR_DestroyConnection();
    if (!N.Deserialize(PayloadPtr, PayloadSize)) {
        X_DEBUG_PRINTF("Invalid protocol");
        return;
    }
    auto CCP = GetConnectionById(N.ProxySideConnectionId);
    if (!CCP) {
        X_DEBUG_PRINTF("ClientConnection not found: %" PRIx64 "", N.ProxySideConnectionId);
        return;
    }
    switch (CCP->Phase) {
        case xPA_ClientConnection::eS5WaitForConnectionEstablish: {
            static constexpr const ubyte Reply[] = {
                '\x05', '\x03', '\x00',          // network unreachable
                '\x01',                          // ipv4
                '\x00', '\x00', '\x00', '\x00',  // ip: 0.0.0.0
                '\x00', '\x00',                  // port 0:
            };
            CCP->PostData(Reply, sizeof(Reply));
            CCP->Phase = xPA_ClientConnection::eS5WaitForConnectionClosed;
            LingerKill(*CCP);
            break;
        }

        case xPA_ClientConnection::eS5ConnectionReady: {
            CCP->Phase = xPA_ClientConnection::eS5WaitForConnectionClosed;
            if (CCP->HasPendingWrites()) {
                LingerKill(*CCP);
            } else {
                Kill(*CCP);
            }
            break;
        }

        case xPA_ClientConnection::eHttpRawWaitForConnectionEstablish: {
            CCP->Phase = xPA_ClientConnection::eHttpRawClosed;
            CCP->PostData("HTTP/1.1 502 Bad Gateway\r\nProxy-agent: proxy / 1.0\r\n\r\n", 64);
            LingerKill(*CCP);
            break;
        }

        case xPA_ClientConnection::eHttpRawReady:
            CCP->Phase = xPA_ClientConnection::eHttpRawClosed;
            if (CCP->HasPendingWrites()) {
                LingerKill(*CCP);
            } else {
                Kill(*CCP);
            }
            break;

        case xPA_ClientConnection::eHttpNormalReady: {
            CCP->Phase = xPA_ClientConnection::eHttpNormalClosed;
            if (CCP->HasPendingWrites()) {
                LingerKill(*CCP);
            } else {
                Kill(*CCP);
            }
            break;
        }

        case xPA_ClientConnection::eHttpNormalWaitForConnectionEstablish: {
            CCP->Phase = xPA_ClientConnection::eHttpNormalClosed;
            CCP->PostData("HTTP/1.1 502 Bad Gateway\r\nProxy-agent: proxy / 1.0\r\n\r\n", 64);
            LingerKill(*CCP);
            break;
        }
        default:
            X_DEBUG_PRINTF("invalid connection state: %u", (unsigned)CCP->Phase);
            Kill(*CCP);
            break;
    }
    return;
}

void xPA_ClientConnectionManager::OnRelaySidePushData(const ubyte * PayloadPtr, size_t PayloadSize) {

    X_DEBUG_PRINTF("size=%zi", PayloadSize);

    auto Push = xPR_PushData();
    if (!Push.Deserialize(PayloadPtr, PayloadSize)) {
        X_DEBUG_PRINTF("invalid protocol");
        return;
    }

    auto CCP = GetConnectionById(Push.ProxySideConnectionId);
    if (!CCP) {
        X_DEBUG_PRINTF("ClientConnection not found: %" PRIx64 "", Push.ProxySideConnectionId);
        return;
    }

    KeepAlive(*CCP);
    CCP->PostData(Push.PayloadView.data(), Push.PayloadView.size());
    TryUpdateDataTransferSize(CCP, 0, PayloadSize);
}

void xPA_ClientConnectionManager::TryUpdateDataTransferSize(xPA_ClientConnection * CCP, uint64_t UploadSizeIncrement, uint64_t DumpedSizeIncrement) {
    CCP->TotalUploadDataSize += UploadSizeIncrement;
    CCP->TotalDumpedDataSize += DumpedSizeIncrement;

    auto UploadDiff = CCP->TotalUploadDataSize - CCP->TotalReportedUploadDataSize;
    auto DumpedDiff = CCP->TotalDumpedDataSize - CCP->TotalReportedDumpedDataSize;
    X_DEBUG_PRINTF("UploadDiff: %" PRIu64 ", DumpedDiff: %" PRIu64 "", UploadDiff, DumpedDiff);

    if (UploadDiff < READ_OVERFLOW_SIZE && DumpedDiff < WRITE_OVERFLOW_SIZE) {

        return;
    }

    auto RCP = GlobalTestRCM.GetConnectionById(CCP->RelayConnectionId);
    if (!RCP) {
        X_DEBUG_PRINTF("Invalid relay server connection: %" PRIx64 "", CCP->RelayConnectionId);
        Kill(*CCP);
        return;
    }

    // do report
    auto N                  = xPR_ConnectionStateNotify();
    N.ProxySideConnectionId = CCP->ConnectionId;
    N.RelaySideConnectionId = CCP->RelaySideConnectionId;
    N.NewState              = xPR_ConnectionStateNotify::STATE_UPDATE_TRANSFER;
    N.TotalUploadedBytes    = CCP->TotalUploadDataSize;
    N.TotalDumpedBytes      = CCP->TotalDumpedDataSize;
    RCP->PostMessage(Cmd_PA_RL_NotifyConnectionState, 0, N);

    CCP->TotalReportedUploadDataSize = CCP->TotalUploadDataSize;
    CCP->TotalReportedDumpedDataSize = CCP->TotalDumpedDataSize;
    X_DEBUG_PRINTF("Upload: %" PRIu64 ", Dumped: %" PRIu64 "", CCP->TotalReportedUploadDataSize, CCP->TotalReportedDumpedDataSize);
}
