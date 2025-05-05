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

void xDR_DeviceContextManager::Tick(uint64_t NowMS) {
    Ticker.Update(NowMS);
    auto KP = NowMS - DEVICE_KEEPALIVE_TIMEOUT_MS;
    while (auto PD = static_cast<xDR_DeviceContext *>(TimeoutDeviceList.PopHead([KP](const xDR_TimeoutNode & N) mutable { return N.TimestampMS <= KP++; }))) {
        RemoveDevice(PD);
    }
}

void xDR_DeviceContextManager::UpdateDevice(const xDR_DeviceInfoBase & InfoBase) {
    auto Iter = DeviceMap.find(InfoBase.DeviceId);
    if (Iter != DeviceMap.end()) {
        KeepAlive(Iter->second);
        return;
    }

    // add new device:
    auto PD      = new xDR_DeviceContext;
    PD->InfoBase = InfoBase;

    CountryDeviceList[InfoBase.CountryId].AddTail(*PD);
    StateDeviceList[InfoBase.StateId].AddTail(*PD);
    CityDeviceList[InfoBase.CityId].AddTail(*PD);

    KeepAlive(Iter->second);
}

void xDR_DeviceContextManager::RemoveDevice(xDR_DeviceContext * Device) {
    RemoveDeviceById(Device->InfoBase.DeviceId);
}

void xDR_DeviceContextManager::RemoveDeviceById(const std::string & DeviceId) {
    auto Iter = DeviceMap.find(DeviceId);
    if (Iter == DeviceMap.end()) {
        return;
    }
    delete Iter->second;
    DeviceMap.erase(Iter);
}

void xDR_DeviceContextManager::KeepAlive(xDR_DeviceContext * Device) {
    Device->TimestampMS = Ticker();
    TimeoutDeviceList.GrabTail(*Device);
}
