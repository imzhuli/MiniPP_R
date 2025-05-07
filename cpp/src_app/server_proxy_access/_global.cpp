#include "./_global.hpp"

#include <config/config.hpp>

xNetAddress              ConfigExportBindAddress;
xNetAddress              ConfigTcpBindAddress;
xNetAddress              ConfigExportUdpBindAddress;
xNetAddress              ConfigUdpBindAddress;
xNetAddress              ConfigAuthServerAddress;
xNetAddress              ConfigDeviceSelectorAddress;
std::vector<xNetAddress> ConfigCenterAddressList;

xRunState                   GlobalRunState;
xTicker                     GlobalTicker;
xPA_AuthCacheManager        GlobalAuthCacheManager;
xPA_RelayConnectionManager  GlobalRelayConnectionManager;
xPA_ClientConnectionManager GlobalClientConnectionManager;
xPA_RelayServerListManager  GlobalRelayServerListManager;
xPA_DeviceSelectorManager   GlobalDeviceSelectorManager;

xPA_LocalAudit GlobalLocalAudit = {};

std::map<uint64_t, xNetAddress> ConfigRelayServerMapForTest;
xPATest_RCM                     GlobalTestRCM;

static void LoadRelayServerMapForTest(const std::string & RelayServerMapStr) {
    Reset(ConfigRelayServerMapForTest);

    auto Segs = Split(RelayServerMapStr, ",");
    for (auto & S : Segs) {
        S = Trim(S);
        if (S.empty()) {
            continue;
        }
        auto ISegs = Split(S, "@");
        if (ISegs.size() != 2) {
            continue;
        }
        auto & IdStr   = ISegs[0];
        auto & AddrStr = ISegs[1];

        X_DEBUG_PRINTF("RelayServerMapSeg: %s / %s", IdStr.c_str(), AddrStr.c_str());

        uint64_t Id   = strtoumax(IdStr.c_str(), nullptr, 10);
        auto     Addr = xNetAddress::Parse(AddrStr);

        if (!Addr || !Addr.Port) {
            continue;
        }
        X_DEBUG_PRINTF("EnabledServerAddress : %" PRIx64 ", @%s", Id, Addr.ToString().c_str());

        ConfigRelayServerMapForTest.insert(std::make_pair(Id, Addr));
    }
}

void LoadConfig(const std::string & filename) {
    auto Loader                     = xel::xConfigLoader(filename.c_str());
    auto ConfigCenterAddressListStr = std::string();

    Loader.Require(ConfigTcpBindAddress, "TcpBindAddress");
    Loader.Require(ConfigCenterAddressListStr, "ConfigCenterAddressList");
    Loader.Require(ConfigDeviceSelectorAddress, "ConfigDeviceSelectorAddress");
    Loader.Require(ConfigAuthServerAddress, "ConfigAuthServerAddress");

    //
    auto CCList = Split(ConfigCenterAddressListStr, ",");
    for (auto & CC : CCList) {
        auto AddrStr = Trim(CC);
        if (AddrStr.empty()) {
            continue;
        }
        auto Addr = xNetAddress::Parse(CC);
        RuntimeAssert(Addr && Addr.Port);

        cout << "Add ConfigCenterAddress: " << Addr.ToString() << endl;
        ConfigCenterAddressList.push_back(Addr);
    }

    std::string RelayServerMapStr;
    Loader.Optional(RelayServerMapStr, "RelayServerMapStr");
    LoadRelayServerMapForTest(RelayServerMapStr);
}
