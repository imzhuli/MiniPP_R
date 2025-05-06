#include "../lib_server_util/all.hpp"

#include <pp_protocol/command.hpp>
#include <pp_protocol/internal/all.hpp>

auto IC  = xIoContext();
auto ICG = xResourceGuard(IC);

static constexpr const xCountryId CN = 1234;

struct xFakeAuthService : xService {

    bool OnClientPacket(xServiceClientConnection & Connection, xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) override {

        if (CommandId != Cmd_AuthService_QueryAuthCache) {
            X_DEBUG_PRINTF("Invalid command id");
            return true;
        }

        auto Req = xQueryAuthCache();
        if (!Req.Deserialize(PayloadPtr, PayloadSize)) {
            X_DEBUG_PRINTF("Invalid request");
            return true;
        }

        auto Resp      = xQueryAuthCacheResp();
        Resp.CountryId = CN;
        PostMessage(Connection, Cmd_AuthService_QueryAuthCacheResp, RequestId, Resp);
        return true;
    }
};
xFakeAuthService FAS;

int main(int argc, char ** argv) {

    return 0;
}
