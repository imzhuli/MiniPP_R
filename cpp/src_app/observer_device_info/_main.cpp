#include "../lib_server_util/all.hpp"
#include "./_global.hpp"

#include <pp_common/_.hpp>
#include <pp_protocol/command.hpp>

struct xODI : public xClientPool {
    using xClientPool::Clean;
    using xClientPool::Init;

    bool OnServerPacket(xClientConnection & CC, xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) override {

        cout << "OnServerPacket: " << endl;
        cout << HexShow(PayloadPtr, PayloadSize) << endl;
        switch (CommandId) {
            case Cmd_DSR_DS_DeviceUpdate:
                cout << "Cmd_DSR_DS_DeviceUpdate" << endl;
                break;
        }

        return true;
    }

    //
};

static auto TargetAddress = xNetAddress();
static auto IC            = xIoContext();
static auto ICG           = xResourceGuard(IC);
static auto ODI           = xODI();

static auto KR = xKfkProducer();

int main(int argc, char ** argv) {

    RuntimeAssert(KR.Init(
        "my-topic",
        {
            { "security.protocol", "SASL_PLAINTEXT" },
            { "sasl.mechanism", "SCRAM-SHA-256" },
            { "sasl.username", "client" },
            { "sasl.password", "client123456" },
            { "bootstrap.servers", "45.197.7.51:9085" },
        }
    ));
    auto KRC = xScopeCleaner(KR);

    // RuntimeAssert(InitKafka());
    // auto KFKG = xScopeGuard(CleanKafka);

    for (size_t I = 0; I < 100; ++I) {
        auto Payload = std::string("Message: ") + std::to_string(I);
        KR.Post(Payload);
    }
    KR.Flush();

    std::this_thread::sleep_for(std::chrono::seconds(3));
    KR.Poll();

    // auto CL = GetConfigLoader(argc, argv);
    // CL.Require(TargetAddress, "TargetAddress");

    // auto ODIG = xResourceGuard(ODI, &IC);
    // RuntimeAssert(ODIG);

    // while (true) {
    //     IC.LoopOnce();
    // }

    return 0;
}
