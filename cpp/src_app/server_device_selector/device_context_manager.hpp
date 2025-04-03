#pragma once
#include <pp_common/base.hpp>
#include <unordered_map>

struct xDR_TimeoutNode : xListNode {
    uint32_t TimestampMS;
};
struct xDR_CountryNode : xListNode {};
struct xDR_StateNode : xListNode {};
struct xDR_CityNode : xListNode {};

struct xDR_DeviceInfoBase {
    uint64_t    ReleayServerAddress;
    uint64_t    DeviceRelaySideKey;
    std::string DeviceId;
};

struct xDR_DeviceContext
    : xDR_TimeoutNode
    , xDR_CountryNode
    , xDR_StateNode
    , xDR_CityNode {
    xDR_DeviceInfoBase BaseInfo;
};

class xDR_DeviceContextManager {
public:
    bool Init();
    void Clean();
    void Tick();

    void UpdateDevice(const xDR_DeviceInfoBase & InfoBase);
    void RemoveDevice(xDR_DeviceContext * Device);
    void RemoveDeviceById(const std::string & DeviceId);

private:
    xTicker                                              Ticker;
    xList<xDR_TimeoutNode>                               TimeoutDeviceList;
    std::unordered_map<uint32_t, xList<xDR_CountryNode>> CountryDeviceList;
    std::unordered_map<uint32_t, xList<xDR_StateNode>>   StateDeviceList;
    std::unordered_map<uint32_t, xList<xDR_CityNode>>    CityDeviceList;
    std::unordered_map<std::string, xDR_DeviceContext *> DeviceMap;
};
