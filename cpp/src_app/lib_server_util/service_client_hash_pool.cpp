#include "./service_client_hash_pool.hpp"

#include <algorithm>

bool xServiceClientHashPool::Init(xIoContext * ICP) {
    this->ICP = ICP;
    return true;
}

void xServiceClientHashPool::Clean() {
    for (auto & P : ActiveServerList) {
        if (P->Connection->IsOpen()) {
            P->Connection->Clean();
        }
        delete X_DEBUG_STEAL(P->Connection);
        delete X_DEBUG_STEAL(P);
    }
    Reset(ActiveServerList);

    for (auto & P : DeleteServerList) {
        if (P->Connection->IsOpen()) {
            P->Connection->Clean();
        }
        delete X_DEBUG_STEAL(P->Connection);
        delete X_DEBUG_STEAL(P);
    }
    Reset(DeleteServerList);

    for (auto & P : UninitializedServerList) {
        assert(!P->Connection);
        delete X_DEBUG_STEAL(P);
    }
    Reset(UninitializedServerList);

    assert(ReconnectServerList.IsEmpty());

    Reset(ICP);
}

bool xServiceClientHashPool::PostMessage(uint64_t Hash, xPacketCommandId CmdId, xPacketRequestId ReqId, xBinaryMessage & Message) {
    if (ActiveServerList.empty()) {
        return false;
    }
    auto CP = ActiveServerList[Hash % ActiveServerList.size()]->Connection;
    assert(CP);
    if (!CP->IsConnected()) {
        return false;
    }
    ubyte Buffer[MaxPacketSize];
    auto  RSize = WriteMessage(Buffer, CmdId, ReqId, Message);
    if (!RSize) {
        return false;
    }
    CP->PostData(Buffer, RSize);
    return true;
}

void xServiceClientHashPool::UpdateServerList(const std::vector<xServerInfo> & NewServerInfoListSource) {
    assert(UninitializedServerList.empty());
    assert(DeleteServerList.empty());
    auto NewServerInfoList = NewServerInfoListSource;
    std::sort(NewServerInfoList.begin(), NewServerInfoList.end(), [](xServerInfo & lhs, xServerInfo & rhs) { return lhs.ServerId < rhs.ServerId; });

    size_t OldIndex = 0;
    size_t NewIndex = 0;
    while (true) {
        if (OldIndex >= ActiveServerList.size()) {
            // add all unprocessed new server:
            for (; NewIndex < NewServerInfoList.size(); ++NewIndex) {
                auto NSP                         = new xServerContext();
                static_cast<xServerInfo &>(*NSP) = NewServerInfoList[NewIndex];
                UninitializedServerList.push_back(NSP);
            }
            break;
        }
        if (NewIndex >= NewServerInfoList.size()) {
            // remove all remain old server
            for (; OldIndex < ActiveServerList.size(); ++OldIndex) {
                auto & S      = ActiveServerList[OldIndex];
                S->DeleteMark = true;
                DeleteServerList.push_back(S);
            }

            break;
        }

        auto & S = ActiveServerList[OldIndex];
        auto & D = NewServerInfoList[NewIndex];
        if (S->ServerId == D.ServerId) {
            if (S->Address != D.Address) {
                S->DeleteMark = true;
                DeleteServerList.push_back(S);

                auto NSP                         = new xServerContext();
                static_cast<xServerInfo &>(*NSP) = D;
                UninitializedServerList.push_back(NSP);
            }
            ++OldIndex;
            ++NewIndex;
            continue;
        } else if (S->ServerId < D.ServerId) {
            // delete:
            S->DeleteMark = true;
            DeleteServerList.push_back(S);
            ++OldIndex;
            continue;
        } else {  // new
            auto NSP                         = new xServerContext();
            static_cast<xServerInfo &>(*NSP) = D;
            UninitializedServerList.push_back(NSP);
            ++NewIndex;
            continue;
        }
    }
    std::erase_if(ActiveServerList, [](auto S) { return S->DeleteMark; });
}

void xServiceClientHashPool::Tick(uint64_t NowMS) {
    Ticker.Update(NowMS);

    //
    if (!UninitializedServerList.empty()) {
        for (auto & P : UninitializedServerList) {
            assert(!P->Connection);
            P->Connection        = new xTcpConnectionEx();
            P->Connection->Owner = P;
            ReconnectServerList.AddTail(*P);
        }

        ActiveServerList.insert(ActiveServerList.end(), UninitializedServerList.begin(), UninitializedServerList.end());
        std::sort(ActiveServerList.begin(), ActiveServerList.end(), [](xServerContext *& lhs, xServerContext *& rhs) { return lhs->ServerId < rhs->ServerId; });
        UninitializedServerList.clear();
    }

    if (!DeleteServerList.empty()) {
        for (auto & P : DeleteServerList) {
            assert(P->Connection);
            if (P->Connection->IsOpen()) {
                P->Connection->Clean();
            }
            delete X_DEBUG_STEAL(P->Connection);
            delete X_DEBUG_STEAL(P);
        }
        DeleteServerList.clear();
    }

    // close old connections:
    ClosingServerList.ForEach([](xServerContext & SC) { SC.Connection->Clean(); });
    ReconnectServerList.GrabListTail(ClosingServerList);

    // do reconnect
    while (auto P = ReconnectServerList.PopHead()) {
        P->Connection->Init(ICP, P->Address, this);
    }
}

void xServiceClientHashPool::OnPeerClose(xTcpConnection * TcpConnectionPtr) {
    auto R = static_cast<xTcpConnectionEx *>(TcpConnectionPtr)->Owner;
    assert(!xListNode::IsLinked(*R));
    ClosingServerList.AddTail(*R);
}

size_t xServiceClientHashPool::OnData(xTcpConnection * TcpConnectionPtr, ubyte * DataPtr, size_t DataSize) {
    return DataSize;
}
