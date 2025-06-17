#include "./_config.hpp"

#include "./_global.hpp"

#include <config/config.hpp>
#include <core/string.hpp>
#include <pp_common/region.hpp>
#include <unordered_map>

void LoadForcedRelayerServers(const std::string & ForcedRelayerConfigFile) {
    if (ForcedRelayerConfigFile.empty()) {
        cout << "INFO: NO ForcedRelayerConfigFile" << endl;
        return;
    }
    cout << "INFO: Reading ForcedRelayers from " << ForcedRelayerConfigFile << endl;
    auto Contents = xel::FileToLines(ForcedRelayerConfigFile);
    for (auto & Line : Contents) {
        auto TL = xel::Split(Line, ",");
        cout << TL.size() << endl;
        for (auto & W : TL) {
            W = xel::Trim(W);
        }
        if (TL.empty() || TL[0][0] == '#') {
            cout << "INFO: Comment linne: " << Line << endl;
            continue;
        }
        if (TL.size() < 2) {
            cerr << "ERROR: Invalid format" << endl;
        }

        auto CtrlAddress = xNetAddress::Parse(TL[0]);
        auto DataAddress = xNetAddress::Parse(TL[1]);
        RuntimeAssert(CtrlAddress && CtrlAddress.Port, "CtrlAddress should be valid address with valid port");
        RuntimeAssert(DataAddress && DataAddress.Port, "DataAddress should be valid address with valid port");

        auto Info        = xCC_RelayServerBase();
        Info.CtrlAddress = CtrlAddress;
        Info.DataAddress = DataAddress;
        ForceRelayServerList.push_back(Info);
    }
}

bool LoadConfig(const char * filename) {
    // LoadForced Relay
    auto Loader = xel::xConfigLoader(filename);

    Loader.Require(BindAddressForDevice, "BindAddressForDevice");
    Loader.Require(BindAddressForDeviceV6Test, "BindAddressForDeviceV6Test");
    Loader.Require(BindAddressForProxyAccess, "BindAddressForProxyAccess");
    Loader.Require(BindAddressForGetBackendServerList, "BindAddressForGetBackendServerList");

    Loader.Require(GeoInfoMapFilename, "GeoInfoMapFilename");
    Loader.Require(IpLocationDbFilename, "IpLocationDbFilename");

    std::string ForcedRelayerConfigFile;
    Loader.Optional(ForcedRelayerConfigFile, "ForcedRelayerConfigFile");
    LoadForcedRelayerServers(ForcedRelayerConfigFile);

    return true;
}
