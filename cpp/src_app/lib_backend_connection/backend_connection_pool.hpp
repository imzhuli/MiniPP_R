#pragma once
#include <map>
#include <pp_common/base.hpp>
#include <server_arch/client_pool.hpp>

class xBackendConnectionPool : protected xClientPool {

public:
    bool Init(xIoContext * ICP, size_t MaxConnectionCount);
    void Clean();
    using xClientPool::Tick;

    uint64_t AddServer(const xNetAddress & Address, const std::string & AppKey, const std::string & AppSecret);
    void     RemoveServer(uint64_t ConnectionId);

    using xClientPool::PostMessage;

protected:
    virtual bool OnBackendPacket(xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) = 0;

private:
    void OnServerConnected(xClientConnection & CC) override;
    bool OnServerPacket(xClientConnection & CC, xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) override;
    void OnServerClose(xClientConnection & CC) override;
    bool OnCmdBackendChallengeResp(xClientConnection & CC, xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize);

    //
    struct xBackendConnectionContext {
        std::string AppKey;
        std::string AppSecret;
        bool        IsChallengeReady = false;
    };
    std::vector<xBackendConnectionContext> ContextList;
};
