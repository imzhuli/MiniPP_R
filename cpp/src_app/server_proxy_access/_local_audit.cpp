#include "./_local_audit.hpp"

#define O(x) OS << #x << "=" << (x) << ' '

std::string xPA_LocalAudit::ToString() const {
    auto OS = std::ostringstream();

    O(BlockedAuthNodeCount);

    O(AuthRequestCountPerMinute);
    O(ActureAuthRequestCountPerMinute);
    O(DeviceSelectionPerMinute);

    O(AuthCacheNodeCount);
    O(OngoingAuthRequestNumber);

    return OS.str();
}

#undef O
