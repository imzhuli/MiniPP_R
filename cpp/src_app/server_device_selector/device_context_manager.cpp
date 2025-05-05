#include "./device_context_manager.hpp"

bool xDS_DeviceContextManager::Init() {
    return true;
}

void xDS_DeviceContextManager::Clean() {
    for (auto & N : DeviceMap) {
        delete N.second;
    }
    Renew(DeviceMap);
}

void xDS_DeviceContextManager::Tick(uint64_t NowMS) {
    Ticker.Update(NowMS);
    auto KP = NowMS - DEVICE_KEEPALIVE_TIMEOUT_MS;
    while (auto PD = static_cast<xDS_DeviceContext *>(TimeoutDeviceList.PopHead([KP](const xDR_TimeoutNode & N) mutable { return N.TimestampMS <= KP++; }))) {
        RemoveDevice(PD);
    }
}

void xDS_DeviceContextManager::UpdateDevice(const xDR_DeviceInfoBase & InfoBase) {
    auto Iter = DeviceMap.find(InfoBase.DeviceId);
    if (Iter != DeviceMap.end()) {
        KeepAlive(Iter->second);
        return;
    }

    // add new device:
    auto PD      = new xDS_DeviceContext;
    PD->InfoBase = InfoBase;

    CountryDeviceList[InfoBase.CountryId].AddTail(*PD);
    StateDeviceList[InfoBase.StateId].AddTail(*PD);
    CityDeviceList[InfoBase.CityId].AddTail(*PD);

    KeepAlive(Iter->second);
}

void xDS_DeviceContextManager::RemoveDevice(xDS_DeviceContext * Device) {
    RemoveDeviceById(Device->InfoBase.DeviceId);
}

void xDS_DeviceContextManager::RemoveDeviceById(const std::string & DeviceId) {
    auto Iter = DeviceMap.find(DeviceId);
    if (Iter == DeviceMap.end()) {
        return;
    }
    delete Iter->second;
    DeviceMap.erase(Iter);
}

void xDS_DeviceContextManager::KeepAlive(xDS_DeviceContext * Device) {
    Device->TimestampMS = Ticker();
    TimeoutDeviceList.GrabTail(*Device);
}

const xDS_DeviceContext * xDS_DeviceContextManager::SelectDeviceByCountryId(xCountryId Id) {
    auto Iter = CountryDeviceList.find(Id);
    if (Iter == CountryDeviceList.end()) {
        return nullptr;
    }
    auto PD = static_cast<xDS_DeviceContext *>(Iter->second.PopHead());
    if (PD) {
        Iter->second.AddTail(*PD);
    }
    return PD;
}

const xDS_DeviceContext * xDS_DeviceContextManager::SelectDeviceByStateId(xStateId Id) {
    auto Iter = StateDeviceList.find(Id);
    if (Iter == StateDeviceList.end()) {
        return nullptr;
    }
    auto PD = static_cast<xDS_DeviceContext *>(Iter->second.PopHead());
    if (PD) {
        Iter->second.AddTail(*PD);
    }
    return PD;
}

const xDS_DeviceContext * xDS_DeviceContextManager::SelectDeviceByCityId(xCityId Id) {
    auto Iter = CityDeviceList.find(Id);
    if (Iter == CityDeviceList.end()) {
        return nullptr;
    }
    auto PD = static_cast<xDS_DeviceContext *>(Iter->second.PopHead());
    if (PD) {
        Iter->second.AddTail(*PD);
    }
    return PD;
}
