#pragma once
#include "./_.hpp"

using namespace xel;

#include <atomic>
#include <cinttypes>
#include <iostream>
#include <mutex>
#include <string>
#include <tuple>

namespace __pp_common_detail__ {
    template <typename T>
    inline void __TickOne__(uint64_t NowMS, T & Target) {
        Target.Tick(NowMS);
    }
    inline void __TickAll__(uint64_t) {  // iteration finishes here
    }
    template <typename T, typename... TOthers>
    inline void __TickAll__(uint64_t NowMS, T & First, TOthers &... Others) {
        __TickOne__(NowMS, First);
        __TickAll__(NowMS, Others...);
    }

}  // namespace __pp_common_detail__
template <typename... T>
inline void TickAll(uint64_t NowMS, T &... All) {
    __pp_common_detail__::__TickAll__(NowMS, All...);
}

// clang-format off
#define CASE_PRINT(x) case x: X_DEBUG_PRINTF("%s", X_STRINGIFY(x)); break

template<typename T>
// pointer to unique_ptr wrapper
std::unique_ptr<T> P2U(T * Ptr) {
    return std::unique_ptr<T>(Ptr);
}

std::string DebugSign(const void * DataPtr, size_t Size);
static inline std::string DebugSign(const std::string_view& V) {
    return DebugSign(V.data(), V.size());
}

extern uint32_t HashString(const char * S);
extern uint32_t HashString(const char * S, size_t Len);
extern uint32_t HashString(const std::string & S);

// clang-format on
