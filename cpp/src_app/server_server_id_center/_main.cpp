#include "../lib_server_util/all.hpp"

#include <pp_common/base.hpp>

static constexpr const auto IdShift       = 44;
static constexpr const auto TimestampMask = (uint64_t(1) << IdShift) - 1;

static auto ConfigServiceBindAddress = xNetAddress();

static auto GlobalTicker = xTicker();
static auto IC           = xIoContext();
static auto ICG          = xResourceGuard(IC);

class xServerIdCenterService : public xService {

public:
    bool Init(xIoContext * IoContextPtr, const xNetAddress & BindAddress) {
        if (!IdManager.Init()) {
            return false;
        }
        if (!xService::Init(IoContextPtr, BindAddress, false, IdManager.MaxObjectId)) {
            IdManager.Clean();
            return false;
        }
        return true;
    }

    void Clean() {
        xService::Clean();
        IdManager.Clean();
    }

    uint64_t Acquire() {
        uint32_t InnerId = IdManager.Acquire();
        if (!InnerId) {
            return 0;
        }
        return (static_cast<uint64_t>(InnerId) << 44) + (TimestampMask & GetTimestampMS());
    }

    uint64_t TryAcquire(uint64_t Id) {
        uint32_t InnerId = (uint32_t)(Id >> 44);
        if (Id) {
            if (!InnerId || InnerId > IdManager.MaxObjectId) {
                X_DEBUG_PRINTF("Invalid previous server id");
                return 0;
            }
        }

        if (!IdManager.MarkInUse(InnerId)) {
            return Acquire();
        }
        return Id;
    }

    void Release(uint64_t Id) {
        uint32_t InnerId = (uint32_t)(Id >> 44);
        IdManager.Release(InnerId);
    }

private:
    xObjectIdManager IdManager;
};

static auto Service = xServerIdCenterService();

int main(int argc, char ** argv) {

    auto CL = GetConfigLoader(argc, argv);
    CL.Require(ConfigServiceBindAddress, "BindAddress");

    auto SG = xResourceGuard(Service, &IC, ConfigServiceBindAddress);

    while (true) {
        GlobalTicker.Update();
        TickAll(GlobalTicker(), Service);
    }

    return 0;
}
