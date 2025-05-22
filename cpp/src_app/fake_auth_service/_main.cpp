#include "../lib_server_util/all.hpp"

#include <pp_common/region.hpp>
#include <pp_protocol/command.hpp>
#include <pp_protocol/internal/all.hpp>

using namespace xel;
using namespace std;

static auto IC     = xIoContext();
static auto ICG    = xResourceGuard(IC);
static auto Ticker = xTicker();

static const xCountryId  BR          = 16978;
static const xNetAddress BindAddress = xNetAddress::Parse("0.0.0.0:17201");

static auto CountryId = BR;

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
        Resp.CountryId = CountryId;
        PostMessage(Connection, Cmd_AuthService_QueryAuthCacheResp, RequestId, Resp);
        return true;
    }

    //
};

xFakeAuthService FAS;

int main(int argc, char ** argv) {

    auto CL = xCommandLine(
        argc, argv,
        {
            { 'n', "nation", "nation", true },
        }
    );

    auto Nopt = CL["nation"];
    if (Nopt() && Nopt->size() == 2) {
        CountryId = CountryCodeToCountryId(Nopt->c_str());
        cout << "using country : " << *Nopt << ": " << CountryId << endl;
    }

    FAS.Init(&IC, BindAddress);

    while (true) {
        Ticker.Update();
        IC.LoopOnce();
        FAS.Tick(Ticker());
    }

    return 0;
}
