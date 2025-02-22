#include <core/core_time.hpp>
#include <crypto/md5.hpp>
#include <pp_common/base.hpp>
#include <pp_protocol/command.hpp>
#include <pp_protocol/config_center/terminal.hpp>

// dont reorder
#include "./_config.hpp"
#include "./_global.hpp"

auto RuntimeEnv = xResourceGuard{ GlobalIoContext };

struct xIpv6TestChennelReactor : xUdpChannel::iListener {
    void OnData(xUdpChannel * ChannelPtr, ubyte * DataPtr, size_t DataSize, const xNetAddress & RemoteAddress) override {
        auto AddrStr = RemoteAddress.ToString();
        cout << "ipv6 chnallenge :" << endl << HexShow(AddrStr) << endl;

        if (DataSize < PacketHeaderSize) {
            X_DEBUG_PRINTF("Invalid packet size");
            return;
        }
        auto Header = xPacketHeader();
        Header.Deserialize(DataPtr);
        if (Header.PacketSize != DataSize) {
            X_DEBUG_PRINTF("Invalid packet size from header");
            return;
        }

        X_DEBUG_PRINTF(
            "Request: CmdId=%" PRIx32 ", RequestId=%" PRIx64 ", RemoteAddress=%s", Header.CommandId, Header.RequestId, RemoteAddress.ToString().c_str()
        );
        auto Payload     = xPacket::GetPayloadPtr(DataPtr);
        auto PayloadSize = Header.GetPayloadSize();
        Touch(PayloadSize);

        X_DEBUG_PRINTF("Request body: \n%s", HexShow(Payload, PayloadSize).c_str());
        switch (Header.CommandId) {
            case Cmd_Terminal_CC_Challenge: {
                OnTerminalChallenge(ChannelPtr, Payload, PayloadSize, RemoteAddress);
                break;
            }

            default:
                X_DEBUG_PRINTF("Unrecognized command");
                break;
        }
        return;
    };

    void OnTerminalChallenge(xUdpChannel * ChannelPtr, const ubyte * Payload, size_t PayloadSize, const xNetAddress & RemoteAddress) {
        auto Request = xCC_DeviceChallenge();
        if (!Request.Deserialize(Payload, PayloadSize)) {
            X_DEBUG_PRINTF("Invalid requst format");
        }

        X_DEBUG_PRINTF("Version:%" PRIu32 ", Timestamp:%" PRIu64 ", Sign=%s", Request.AppVersion, Request.Timestamp, Request.Sign.c_str());
        // check sign:
        auto Source = "TLMPP1" + std::to_string(Request.Timestamp);
        auto Digest = Md5(Source.data(), Source.size());
        if (Request.Sign != StrToHex(Digest.Data(), Digest.Size())) {
            X_DEBUG_PRINTF("Invalid sign");
            return;
        }

        X_DEBUG_PRINTF("Challenge accepted");
        auto Resp            = xCC_DeviceChallengeResp();
        Resp.TerminalAddress = RemoteAddress.Ip();

        ubyte Buffer[MaxPacketSize];
        auto  RSize = WritePacket(Cmd_Terminal_CC_ChallengeResp, 0, Buffer, Resp);

        X_DEBUG_PRINTF("Post Response to %s\n%s", RemoteAddress.ToString().c_str(), HexShow(Buffer, RSize).c_str());
        ChannelPtr->PostData(Buffer, RSize, RemoteAddress);
    }
} Ipv6TestChennelReactor;

struct xChallengeChennelReactor : xUdpChannel::iListener {

    void OnData(xUdpChannel * ChannelPtr, ubyte * DataPtr, size_t DataSize, const xNetAddress & RemoteAddress) override {
        auto AddrStr = RemoteAddress.ToString();
        cout << "ipv6 chnallenge :" << endl << HexShow(AddrStr) << endl;

        if (DataSize < PacketHeaderSize) {
            X_DEBUG_PRINTF("Invalid packet size");
            return;
        }
        auto Header = xPacketHeader();
        Header.Deserialize(DataPtr);
        if (Header.PacketSize != DataSize) {
            X_DEBUG_PRINTF("Invalid packet size from header");
            return;
        }

        X_DEBUG_PRINTF(
            "Request: CmdId=%" PRIx32 ", RequestId=%" PRIx64 ", RemoteAddress=%s", Header.CommandId, Header.RequestId, RemoteAddress.ToString().c_str()
        );
        auto Payload     = xPacket::GetPayloadPtr(DataPtr);
        auto PayloadSize = Header.GetPayloadSize();
        Touch(PayloadSize);

        X_DEBUG_PRINTF("Request body: \n%s", HexShow(Payload, PayloadSize).c_str());
        switch (Header.CommandId) {
            case Cmd_Terminal_CC_Challenge: {
                OnTerminalChallenge(ChannelPtr, Payload, PayloadSize, RemoteAddress);
                break;
            }

            default:
                X_DEBUG_PRINTF("Unrecognized command");
                break;
        }
        return;
    };

    void OnTerminalChallenge(xUdpChannel * ChannelPtr, const ubyte * Payload, size_t PayloadSize, const xNetAddress & RemoteAddress) {
        auto AddrStr = RemoteAddress.ToString();
        cout << "chnallenge data:" << endl << HexShow(AddrStr) << endl;

        auto Request = xCC_DeviceChallenge();
        if (!Request.Deserialize(Payload, PayloadSize)) {
            X_DEBUG_PRINTF("Invalid requst format");
        }

        X_DEBUG_PRINTF("Version:%" PRIu32 ", Timestamp:%" PRIu64 ", Sign=%s", Request.AppVersion, Request.Timestamp, Request.Sign.c_str());
        // check sign:
        auto Source = "TLMPP1" + std::to_string(Request.Timestamp);
        auto Digest = Md5(Source.data(), Source.size());
        if (Request.Sign != StrToHex(Digest.Data(), Digest.Size())) {
            X_DEBUG_PRINTF("Invalid sign");
            return;
        }

        X_DEBUG_PRINTF("Challenge accepted, checking relay server nodes");
        auto RID = IpLocationManager.GetRegionByIp(RemoteAddress.ToString().c_str());
        auto RSI = RelayServerManager.GetRelayServerByRegion(RID);

        auto Resp            = xCC_DeviceChallengeResp();
        Resp.TerminalAddress = RemoteAddress.Ip();
        if (RSI) {
            Resp.CtrlAddress = RSI->CtrlAddress;
            Resp.DataAddress = RSI->DataAddress;
            Resp.CheckKey    = "TLMPP-FOR-TEST";
        }

        ubyte Buffer[MaxPacketSize];
        auto  RSize = WritePacket(Cmd_Terminal_CC_ChallengeResp, 0, Buffer, Resp);

        X_DEBUG_PRINTF("Post Response to %s\n%s", RemoteAddress.ToString().c_str(), HexShow(Buffer, RSize).c_str());
        ChannelPtr->PostData(Buffer, RSize, RemoteAddress);
    };
} ChallengeChennelReactor;

void SetupIpv6TestChannel() {
    if (!BindAddressForDeviceV6Test) {
        cout << "no BindAddressForDeviceV6Test found" << endl;
        return;
    }
    Ipv6TestChennel.Init(&GlobalIoContext, BindAddressForDeviceV6Test, &Ipv6TestChennelReactor);
}

bool SetupChallengeChannel() {
    return ChallengeChannel.Init(&GlobalIoContext, BindAddressForDevice, &ChallengeChennelReactor);
}

void CleanChallengeChannel() {
    ChallengeChannel.Clean();
}

int main(int argc, char ** argv) {

    auto CL = xCommandLine(
        argc, argv,
        {
            { 'c', nullptr, "config_file", true },
        }
    );

    auto ConfigFileOpt = CL["config_file"];
    RuntimeAssert(ConfigFileOpt());

    LoadConfig(ConfigFileOpt->c_str());

    auto ILG  = xResourceGuard(IpLocationManager, IpLocationDbFilename);
    auto RSMG = xResourceGuard(RelayServerManager);
    auto CCMG = xResourceGuard(ChallengeContextManager);

    RuntimeAssert(RuntimeEnv);
    SetupIpv6TestChannel();
    RuntimeAssert(SetupChallengeChannel());

    // for test:
    auto TestRelay        = xCC_RelayServerBase();
    TestRelay.CtrlAddress = xNetAddress::Parse("192.168.5.116:17000");
    TestRelay.DataAddress = xNetAddress::Parse("192.168.5.116:17001");
    auto Key              = RelayServerManager.AddServerInfo(TestRelay);
    cout << "TestRelayServerKey: " << StrToHex(&Key, 8) << endl;
    RelayServerManager.RemoveServerInfoByKey(Key);
    Key = RelayServerManager.AddServerInfo(TestRelay);
    cout << "TestRelayServerKey: " << StrToHex(&Key, 8) << endl;

    auto Ticker = xTicker();
    while (true) {
        Ticker.Update();
        GlobalIoContext.LoopOnce();
        TickAll(Ticker(), IpLocationManager, RelayServerManager, ChallengeContextManager);
    }

    GlobalIoContext.Clean();

    return 0;
}
