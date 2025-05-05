#include "./_config.hpp"

#include "./_global.hpp"

#include <config/config.hpp>
#include <core/uuid.hpp>

size32_t MaxDeviceCount          = 10'0000;
size32_t MaxProxyCount           = 3000;
size32_t MaxRelayConnectionCount = 30'0000;

bool LoadConfig(const char * filename) {

    auto RandomUuid = xUuid(xGeneratorInit());

    // LoadForced Relay
    auto Loader = xel::xConfigLoader(filename);
    Loader.Optional(ServerUuid, "ServerUuid", RandomUuid.ToString());

    Loader.Require(BindCtrlAddress, "BindCtrlAddress");
    Loader.Require(BindDataAddress, "BindDataAddress");
    Loader.Require(BindProxyAddress, "BindProxyAddress");

    Loader.Require(ExportCtrlAddress, "ExportCtrlAddress");
    Loader.Require(ExportDataAddress, "ExportDataAddress");
    Loader.Require(ExportProxyAddress, "ExportProxyAddress");

    Loader.Require(DeviceAuditAddress, "DeviceAuditAddress");

    return true;
}
