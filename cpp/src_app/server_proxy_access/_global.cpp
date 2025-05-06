#include "./_global.hpp"

#include <config/config.hpp>

xNetAddress              ConfigExportBindAddress;
xNetAddress              ConfigTcpBindAddress;
xNetAddress              ConfigExportUdpBindAddress;
xNetAddress              ConfigUdpBindAddress;
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

void LoadConfig(const std::string & filename) {
    auto Loader                     = xel::xConfigLoader(filename.c_str());
    auto ConfigCenterAddressListStr = std::string();

    Loader.Require(ConfigTcpBindAddress, "TcpBindAddress");
    Loader.Require(ConfigCenterAddressListStr, "ConfigCenterAddressList");
    Loader.Require(ConfigDeviceSelectorAddress, "ConfigDeviceSelectorAddress");

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
}
