#pragma once
#include <map>
#include <object/object.hpp>
#include <pp_common/base.hpp>
#include <pp_common/region.hpp>

struct xCC_ChallengeContext : public xListNode {

    //
};

class xCC_ChallengeContextManager {
public:
    bool Init();
    void Clean();

    void Tick(uint64_t NowMS);

    //
private:
    static constexpr const size_t DefaultChallengePoolSize = 8000;

    uint64_t                              NowMS = 0;
    xIndexedStorage<xCC_ChallengeContext> ChallengeContextPool;
};
