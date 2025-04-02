#pragma once
#include <map>
#include <object/object.hpp>
#include <pp_common/base.hpp>
#include <pp_common/region.hpp>

//
#include <array>

using xCC_RelayServerKey = uint64_t;

struct xCC_RelayServerInfo_RegionAllNode : xListNode {};
struct xCC_RelayServerInfo_RegionContinentNode : xListNode {};
struct xCC_RelayServerInfo_RegionContryNode : xListNode {};
struct xCC_RelayServerInfo_RegionStateNode : xListNode {};
struct xCC_RelayServerInfo_RegionCityNode : xListNode {};

struct xCC_RelayServerBase {
    xCC_RelayServerKey Key         = 0;
    xNetAddress        CtrlAddress = {};
    xNetAddress        DataAddress = {};
};

struct xCC_RelayServerInfo
    : xCC_RelayServerInfo_RegionAllNode
    , xCC_RelayServerInfo_RegionContinentNode
    , xCC_RelayServerInfo_RegionContryNode {
    //
    xCC_RelayServerBase Base;
};

class xCC_RelayServerManager {

public:
    bool Init();
    void Clean();
    void Tick(uint64_t NowMS);

    auto AddServerInfo(const xCC_RelayServerBase & NewInfo) -> xCC_RelayServerKey;
    auto RemoveServerInfoByKey(xCC_RelayServerKey Key) -> void;

    //
    const xCC_RelayServerBase * GetAnyRelayServer();
    const xCC_RelayServerBase * GetRelayServerByRegion(const xRegionId & Rid);

private:
    xList<xCC_RelayServerInfo_RegionAllNode>                               RelayServerListAll;
    std::map<xContinentId, xList<xCC_RelayServerInfo_RegionContinentNode>> ContinentListMap;
    std::map<xCountryId, xList<xCC_RelayServerInfo_RegionContryNode>>      ContryListMap;

    static constexpr const size_t                        RelayServerPoolSize = 4096;
    std::array<xCC_RelayServerInfo, RelayServerPoolSize> RelayServerPool;
    xObjectIdManagerMini                                 IdManager;
    //
};
