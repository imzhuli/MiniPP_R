#include "./server_id_manager.hpp"

bool xServerIdManager::Init() {
    if (!IdManager.Init()) {
        return false;
    }
    auto IMC = xScopeCleaner(IdManager);

    assert(!RandomPool);
    RandomPool = new (std::nothrow) uint32_t[IdManager.MaxObjectId];
    if (!RandomPool) {
        return false;
    }
    ZeroFill(RandomPool, IdManager.MaxObjectId);

    std::random_device rd;
    RandomGeneratorHolder.CreateValue(rd());

    IMC.Dismiss();
    return true;
}

void xServerIdManager::Clean() {
    RandomGeneratorHolder.Destroy();
    delete[] Steal(RandomPool);
    auto IMC = xScopeCleaner(IdManager);
    return;
}

uint32_t xServerIdManager::GenerateRandom() {
    return RandomDistribution(*RandomGeneratorHolder) ^ 0x784C6565u;
}

uint64_t xServerIdManager::AcquireServerId() {
    auto Id32 = IdManager.Acquire();
    if (!Id32) {
        return 0;
    }
    auto Random32        = GenerateRandom();
    RandomPool[Id32 - 1] = Random32;
    return Make64(Id32, Random32);
}

uint64_t xServerIdManager::RegainServerId(uint64_t ServerId) {
    auto Id       = High32(ServerId);
    auto Random32 = Low32(ServerId);
    if (!Id || Id > IdManager.MaxObjectId || !Random32) {
        X_DEBUG_PRINTF("invalide previous ServerId");
        return AcquireServerId();
    }
    auto & RR = RandomPool[Id - 1];
    if (!RR) {
        X_DEBUG_PRINTF("regain server id from unused slot");
        X_RUNTIME_ASSERT(IdManager.Acquire(Id));
        RR = Random32;
        return ServerId;
    }
    return AcquireServerId();
}

bool xServerIdManager::ReleaseServerId(uint64_t ServerId) {
    auto Id       = High32(ServerId);
    auto Random32 = Low32(ServerId);
    if (!Id || Id > IdManager.MaxObjectId) {
        return false;
    }
    auto & RR = RandomPool[Id - 1];
    if (RR != Random32) {
        return false;
    }
    IdManager.Release(Id);
    RR = 0;
    return true;
}
