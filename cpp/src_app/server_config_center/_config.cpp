#include "./_config.hpp"

#include "./_global.hpp"

#include <config/config.hpp>
#include <core/string.hpp>
#include <pp_common/region.hpp>
#include <unordered_map>

bool LoadConfig(const char * filename) {
    // LoadForced Relay
    auto Loader = xel::xConfigLoader(filename);

    Loader.Require(BindAddressForDevice, "BindAddressForDevice");
    Loader.Require(BindAddressForDeviceV6Test, "BindAddressForDeviceV6Test");
    Loader.Require(IpLocationDbFilename, "IpLocationDbFilename");

    return true;
}
