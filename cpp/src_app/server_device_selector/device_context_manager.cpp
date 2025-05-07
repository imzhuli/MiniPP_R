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
    X_DEBUG_PRINTF("");

    auto Iter = DeviceMap.find(InfoBase.DeviceId);
    if (Iter != DeviceMap.end()) {
        auto PD = Iter->second;
        X_DEBUG_PRINTF("UpdateDevice: %s, %u/%u/%u", InfoBase.DeviceId.c_str(), InfoBase.CountryId, InfoBase.StateId, InfoBase.CityId);
        KeepAlive(PD);
        return;
    }

    // add new device:
    auto PD      = new xDS_DeviceContext();
    PD->InfoBase = InfoBase;

    CountryDeviceList[InfoBase.CountryId].AddTail(*PD);
    StateDeviceList[InfoBase.StateId].AddTail(*PD);
    CityDeviceList[InfoBase.CityId].AddTail(*PD);

    DeviceMap[InfoBase.DeviceId] = PD;
    KeepAlive(PD);

    X_DEBUG_PRINTF("UpdateDevice: %s,%" PRIx64 ": %u/%u/%u", InfoBase.DeviceId.c_str(), InfoBase.RelaySideDeviceId, InfoBase.CountryId, InfoBase.StateId, InfoBase.CityId);
}

void xDS_DeviceContextManager::RemoveDevice(xDS_DeviceContext * Device) {
    RemoveDeviceById(Device->InfoBase.DeviceId);
}

void xDS_DeviceContextManager::RemoveDeviceById(const std::string & DeviceId) {
    auto Iter = DeviceMap.find(DeviceId);
    if (Iter == DeviceMap.end()) {
        X_DEBUG_PRINTF("Device not found: %s", DeviceId.c_str());
        return;
    }
    auto DP = Iter->second;
    X_DEBUG_PRINTF("DeviceId=%s, DeviceRuntimeKey=%" PRIx64 "", DeviceId.c_str(), DP->InfoBase.RelaySideDeviceId);

    delete DP;
    DeviceMap.erase(Iter);
}

void xDS_DeviceContextManager::KeepAlive(xDS_DeviceContext * Device) {
    X_DEBUG_PRINTF("%s", Device->InfoBase.DeviceId.c_str());
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
