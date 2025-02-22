#include "./relay_server_manager.hpp"

bool xCC_RelayServerManager::Init() {
    RuntimeAssert(IdManager.Init());
    for (auto & R : RelayServerPool) {
        Renew(R);
    }
    return true;
}

void xCC_RelayServerManager::Clean() {
    for (auto & R : RelayServerPool) {
        Renew(R);
    }
    IdManager.Clean();
}

void xCC_RelayServerManager::Tick(uint64_t NowMS) {
    // TODO
}

const xCC_RelayServerBase * xCC_RelayServerManager::GetAnyRelayServer() {
    auto H = static_cast<xCC_RelayServerInfo *>(RelayServerListAll.PopHead());
    if (!H) {
        return nullptr;
    }
    RelayServerListAll.AddTail(*H);
    return &H->Base;
}

const xCC_RelayServerBase * xCC_RelayServerManager::GetRelayServerByRegion(const xRegionId & Rid) {
    return GetAnyRelayServer();
}

auto xCC_RelayServerManager::AddServerInfo(const xCC_RelayServerBase & NewInfo) -> xCC_RelayServerKey {
    assert(NewInfo.Key == 0);  // dont reuse old data

    auto Id = IdManager.Acquire();
    if (!Id) {
        return 0;
    }
    if (Id > RelayServerPool.size()) {
        IdManager.Release(Id);
        return 0;
    }
    //
    auto   Index  = Id - 1;
    auto   Ramdom = Low32(GetTimestampMS());
    auto   Key    = Make64(Ramdom, Index);
    auto & Ref    = RelayServerPool[Index];

    assert(Ref.Base.Key == 0);
    Ref.Base     = NewInfo;
    Ref.Base.Key = Key;
    RelayServerListAll.AddTail(Ref);

    return Key;
}

auto xCC_RelayServerManager::RemoveServerInfoByKey(xCC_RelayServerKey Key) -> void {
    auto Index = Low32(Key);
    auto Id    = Index + 1;
    if (Index >= RelayServerPool.size()) {
        X_DEBUG_PRINTF("invalid index");
        return;
    }
    auto & Ref = RelayServerPool[Index];
    if (Ref.Base.Key != Key) {
        X_DEBUG_PRINTF("server key does not match");
        return;
    }
    Renew(Ref);
    IdManager.Release(Id);
    return;
}
