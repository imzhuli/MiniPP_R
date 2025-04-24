#pragma once
#include <pp_common/base.hpp>

class xPA_AuthCache {
public:
    bool Init(xIoContext * ICP, const xNetAddress & CCAddress);
    void Clean();
};
