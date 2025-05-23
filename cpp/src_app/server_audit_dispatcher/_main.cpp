#include <pp_common/base.hpp>
#include <server_arch/service.hpp>

static constexpr size_t MAX_PRODUCER_COUNT = 1000;
static constexpr size_t MAX_OBSERVER_COUNT = 100;

static auto ProducerAddress = xNetAddress();
static auto ObserverAddress = xNetAddress();
static auto ObserverIds     = std::vector<xIndexId>();

static void Usage() {
    cerr << "program -p ProducerBindAddress -o ObserverAddress" << endl;
}

struct xAD_Observer : xService {

    void OnClientConnected(xServiceClientConnection & Connection) {
        auto Id = Connection.GetConnectionId();
        ObserverIds.push_back(Id);
    }

    void OnClientClose(xServiceClientConnection & Connection) {
        auto Id  = Connection.GetConnectionId();
        auto End = ObserverIds.end();
        for (auto Iter = ObserverIds.begin(); Iter != End; ++Iter) {
            if (*Iter == Id) {
                ObserverIds.erase(Iter);
                return;
            }
        }
        Unreachable();
    }

    //
};

static auto IC       = xIoContext();
static auto Observer = xAD_Observer();

struct xAD_Producer : xService {

    bool OnClientPacket(xServiceClientConnection & Connection, xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) override {
        if (ObserverIds.empty()) {
            return true;
        }

        ubyte Buffer[MaxPacketSize];
        auto  RSize = BuildPacket(Buffer, CommandId, RequestId, PayloadPtr, PayloadSize);
        for (auto Id : ObserverIds) {
            Observer.PostData(Id, Buffer, RSize);
        }
        return true;
    }

    //
};
static auto Producer = xAD_Producer();

int main(int argc, char ** argv) {

    auto CL = xCommandLine(
        argc, argv,
        {
            { 'p', nullptr, "producer", true },
            { 'o', nullptr, "observer", true },
        }
    );

    auto OP = CL["producer"];
    auto OO = CL["observer"];

    if (!OP() || !OO()) {
        Usage();
        return -1;
    }

    ProducerAddress = xNetAddress::Parse(*OP);
    ObserverAddress = xNetAddress::Parse(*OO);

    if (!ProducerAddress || !ProducerAddress.Port) {
        cerr << "invalid producer bind address" << endl;
        return -1;
    }
    if (!ObserverAddress || !ObserverAddress.Port) {
        cerr << "invalid observer bind address" << endl;
        return -1;
    }

    auto ICG = xResourceGuard(IC);
    auto DG  = xResourceGuard(Producer, &IC, ProducerAddress, MAX_PRODUCER_COUNT);
    auto OG  = xResourceGuard(Observer, &IC, ObserverAddress, MAX_OBSERVER_COUNT);

    auto Ticker = xTicker();
    while (true) {
        IC.LoopOnce();
        TickAll(Ticker.Update(), Producer, Observer);
    }

    return 0;
}
