#include <pp_component/server_list/server_list.hpp>
#include <server_arch/service.hpp>

auto IC = xIoContext();
struct xRDSL : xService {

    struct xRelayServerContext {
        uint64_t ConnectionId = 0;
    };

    void OnClientConnected(xServiceClientConnection & Connection) {
        auto RSC          = new xRelayServerContext();
        RSC->ConnectionId = Connection.GetConnectionId();
        Connection.SetUserContext({ .P = RSC });
    }
    void OnClientClose(xServiceClientConnection & Connection) {
        auto RSC = (xRelayServerContext *)Connection.GetUserContext().P;
        delete RSC;
    }
    bool OnPacket(xServiceClientConnection & Connection, const xPacketHeader & Header, ubyte * PayloadPtr, size_t PayloadSize) {
        return true;
    }
};
auto TS = xRDSL();

int main(int argc, char ** argv) {
    auto GL = xCommandLine(
        argc, argv,
        {
            { 'b', nullptr, "bind-address", true },
        }
    );

    auto OptB = GL["bind-address"];
    if (!OptB()) {
        cerr << "invalid bind-address" << endl;
        return -1;
    }
    auto BA = xNetAddress::Parse(*OptB);
    if (!BA || !BA.Port) {
        cerr << "invalid bind-address" << endl;
        return -1;
    }

    auto ICG = xResourceGuard(IC);
    auto TSG = xResourceGuard(TS, &IC, BA, 4096, true);

    while (true) {
        IC.LoopOnce();
        TS.Tick();
    }

    return 0;
}
