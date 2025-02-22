#include "./challenge.hpp"

bool xCC_ChallengeContextManager::Init() {
    Reset(NowMS);
    return ChallengeContextPool.Init(DefaultChallengePoolSize);
}

void xCC_ChallengeContextManager::Clean() {
    //
}

void xCC_ChallengeContextManager::Tick(uint64_t NowMS) {
    this->NowMS = NowMS;
}
