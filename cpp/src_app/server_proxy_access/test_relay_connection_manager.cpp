#include "./test_relay_connection_manager.hpp"

void xPATest_RCM::Tick(uint64_t NowMS) {
    for (auto & [K, V] : ClientMap) {
        Ignore(K);
        V->Tick(NowMS);
    }
}

bool xPATest_RCM::AddRelayServer(uint64_t Id, const xNetAddress Addr) {
    auto & V = ClientMap[Id];
    if (V) {
        return false;
    }
    auto CP = new xClient();
    RuntimeAssert(CP->Init(ICP, Addr));
    V = CP;
    return true;
}

xClient * xPATest_RCM::GetConnectionById(uint64_t Id) {
    auto Iter = ClientMap.find(Id);
    if (Iter == ClientMap.end()) {
        return nullptr;
    }
    return Iter->second;
}
