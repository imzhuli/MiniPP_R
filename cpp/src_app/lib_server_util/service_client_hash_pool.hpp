#pragma once
#include <compare>
#include <pp_common/base.hpp>

class xServiceClientHashPool : xTcpConnection::iListener {
public:
    struct xServerInfo;
    struct xTcpConnectionEx;
    struct xServerContext;

    struct xServerInfo {
        uint64_t    ServerId = {};
        xNetAddress Address  = {};
    };
    struct xTcpConnectionEx : xTcpConnection {
        xServerContext * Owner;
    };
    struct xServerContext
        : xServerInfo
        , xListNode {
        xTcpConnectionEx * Connection = nullptr;
        bool               DeleteMark = false;
    };
    using xServerContextList = xList<xServerContext>;

public:
    bool Init(xIoContext * ICP);
    void Clean();
    void Tick(uint64_t NowMS);

    bool PostMessage(uint64_t Hash, xPacketCommandId CmdId, xPacketRequestId ReqId, xBinaryMessage & Message);
    void UpdateServerList(const std::vector<xServerInfo> & NewServerList);

protected:
    void   OnPeerClose(xTcpConnection * TcpConnectionPtr) override;
    size_t OnData(xTcpConnection * TcpConnectionPtr, ubyte * DataPtr, size_t DataSize) override;

public:
    xTicker                       Ticker;
    xIoContext *                  ICP = nullptr;
    std::vector<xServerContext *> ActiveServerList;
    std::vector<xServerContext *> UninitializedServerList;
    std::vector<xServerContext *> DeleteServerList;
    xServerContextList            ClosingServerList;
    xServerContextList            ReconnectServerList;
};
