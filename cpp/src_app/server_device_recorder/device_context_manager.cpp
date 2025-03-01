#include "./device_context_manager.hpp"

bool xDR_DeviceContextManager::Init() {
    return true;
}

void xDR_DeviceContextManager::Clean() {
    for (auto & N : DeviceMap) {
        delete N.second;
    }
    Renew(DeviceMap);
}
