#pragma once
#include <pp_common/base.hpp>

class xBackendConnection : xel::xTcpConnection::iListener {
private:
    enum struct eState {
        Unspecified = 0,
        SetupReady  = 1,
        Connecting  = 2,
        Connected   = 3,
        Challenging = 4,
        Ready       = 5,
        Closing     = 6,
    };

public:
    bool Init(xIoContext * ICP, const xNetAddress & BackendAddress, const std::string & AppKey, const std::string & AppSecret);
    void Clean();
    void Tick();
    void Tick(uint64_t NowMS);

private:
    void   OnTick();
    void   OnConnected(xTcpConnection * TcpConnectionPtr) override;
    void   OnPeerClose(xTcpConnection * TcpConnectionPtr) override;
    size_t OnData(xTcpConnection * TcpConnectionPtr, ubyte * DataPtr, size_t DataSize) override;

    bool OnPacket(xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize);
    bool OnCmdBackendChallengeResp(xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize);

protected:
    virtual bool OnConnectionReady();
    virtual bool OnBackendPacket(xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) {
        X_DEBUG_PRINTF("");
        return true;
    }

public:
    bool PostData(const void * Data, size_t DataSize);
    bool PostMessage(xPacketCommandId CmdId, xPacketRequestId RequestId, xBinaryMessage & Message);

private:
    static constexpr const uint64_t MinRequestKeepAliveTimeoutMS = 60'000;
    static constexpr const uint64_t MinKeepAliveTimeoutMS        = MinRequestKeepAliveTimeoutMS + 5'000;
    static constexpr const uint64_t MinConnectionTimeoutMS       = 5'000;
    static constexpr const uint64_t MinReconnectTimeoutMS        = 3'000;

    eState  State  = eState::Unspecified;
    xTicker Ticker = xTicker();

    xel::xIoContext *   ICP;
    xel::xTcpConnection Connection;
    xel::xNetAddress    TargetAddress;
    uint64_t            ConnectionStartTimestampMS;
    uint64_t            LastRequestKeepAliveTimestampMS;
    uint64_t            LastKeepAliveTimestampMS;

    // challenge data:
    std::string AppKey;
    std::string AppSecret;

    //
};
