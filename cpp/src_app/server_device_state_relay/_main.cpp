#include "../lib_server_util/all.hpp"
#include "./_global.hpp"

#include <pp_protocol/command.hpp>
#include <server_arch/service.hpp>

using namespace xel;

class xObserverService;
class xProducerService;

class xObserverService : xService {
public:
    using xService::Init;
    void Clean() {
        xService::Clean();
        Reset(Connections);
    }
    using xService::Tick;

    void OnClientConnected(xServiceClientConnection & Connection) override {
        Connections.push_back(&Connection);
    }

    void OnClientClose(xServiceClientConnection & Connection) override {
        auto I = Connections.begin();
        auto E = Connections.end();
        while (I != E) {
            if (*I == &Connection) {
                Connections.erase(I);
                break;
            }
            ++I;
        }
    }

    void DispatchData(const void * DataPtr, size_t DataSize) {
        X_DEBUG_PRINTF("Dispatching data:\n%s", HexShow(DataPtr, DataSize).c_str());
        for (auto PC : Connections) {
            X_DEBUG_PRINTF("ToConnection:%" PRIx64 "", PC->GetConnectionId());
            PC->PostData(DataPtr, DataSize);
        }
    }

private:
    std::vector<xServiceClientConnection *> Connections;

    //
};

static xObserverService OS;

class xProducerService : xService {
public:
    using xService::Clean;
    using xService::Init;
    using xService::Tick;

    bool OnClientPacket(xServiceClientConnection & Connection, xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) {
        X_DEBUG_PRINTF("CommandId: %" PRIx32 ", RequestId:%" PRIx64 ":  \n%s", CommandId, RequestId, HexShow(PayloadPtr, PayloadSize).c_str());
        switch (CommandId) {
            case Cmd_DSR_DS_DeviceOnline:
            case Cmd_DSR_DS_DeviceOffline: {
                ubyte  B[MaxPacketSize];
                size_t RS = BuildPacket(B, CommandId, 0, PayloadPtr, PayloadSize);
                assert(RS);
                OS.DispatchData(B, RS);
            } break;

            default: {
                X_DEBUG_PRINTF("CommandId: %" PRIx32 ", RequestId:%" PRIx64 ":  \n%s", CommandId, RequestId, HexShow(PayloadPtr, PayloadSize).c_str());
                break;
            }
        }

        return true;
    }

    //
};

static xProducerService PS;

static auto IC     = xIoContext();
static auto ICG    = xResourceGuard(IC);
static auto Ticker = xTicker();

int main(int argc, char ** argv) {

    auto CL = GetConfigLoader(argc, argv);
    CL.Require(BindObserverAddress, "BindObserverAddress");
    CL.Require(BindProducerAddress, "BindProducerAddress");

    RuntimeAssert(OS.Init(&IC, BindObserverAddress));
    RuntimeAssert(PS.Init(&IC, BindProducerAddress));

    while (true) {
        Ticker.Update();
        IC.LoopOnce();
        OS.Tick(Ticker());
        PS.Tick(Ticker());
    }

    return 0;
}
