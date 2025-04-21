#include "./_global.hpp"

#include <config/config.hpp>

xNetAddress              ConfigExportBindAddress;
xNetAddress              ConfigTcpBindAddress;
xNetAddress              ConfigExportUdpBindAddress;
xNetAddress              ConfigUdpBindAddress;
xNetAddress              ConfigConfigCenterAddress;
std::vector<uint32_t>    ConfigLocalTagHashList;
std::vector<xNetAddress> ConfigCenterAddressList;

xRunState                   GlobalRunState;
xTicker                     GlobalTicker;
xPA_RelayConnectionManager  GlobalRelayConnectionManager;
xPA_ClientConnectionManager GlobalClientConnectionManager;
xPA_RelayServerListManager  GlobalRelayServerListManager;

void LoadConfig(const std::string & filename) {
    auto Loader                     = xel::xConfigLoader(filename.c_str());
    auto ConfigCenterAddressListStr = std::string();

    Loader.Require(ConfigCenterAddressListStr, "ConfigCenterAddressList");
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
