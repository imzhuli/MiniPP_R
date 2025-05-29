#include "../lib_server_util/all.hpp"

#include <pp_common/base.hpp>
#include <server_arch/service.hpp>

auto IC           = xIoContext();
auto ICG          = xResourceGuard(IC);
auto GlobalTicker = xTicker();

auto ProducerAddress = xNetAddress();
auto ConsumerAddress = xNetAddress();

auto ChallengingServerList = std::vector<xIndexId>();
auto ServerList            = std::vector<xServiceClientConnection *>();

namespace nConnectionFlags {
    static constexpr const uint64_t NEW_CONNECTION = static_cast<uint64_t>(0x00) << 48;
    static constexpr const uint64_t CHALLENGING    = static_cast<uint64_t>(0x01) << 48;
    static constexpr const uint64_t READY          = static_cast<uint64_t>(0x02) << 48;

    static constexpr const uint64_t FLAG_MASK = static_cast<uint64_t>(0xFFU) << 48;
    static constexpr const uint64_t TIME_MASK = static_cast<uint64_t>(-1) & (!FLAG_MASK);

};  // namespace nConnectionFlags

uint64_t GetConnectionFlags(xServiceClientConnection & Connection) {
    return Connection.GetUserContext().U64 & nConnectionFlags::FLAG_MASK;
}

uint64_t GetConnectionTimestamp(xServiceClientConnection & Connection) {
    return Connection.GetUserContext().U64 & nConnectionFlags::TIME_MASK;
}

uint64_t SetConnectionTimestamp(xServiceClientConnection & Connection, uint64_t TimestampMS) {
    return Connection.GetUserContext().U64 = (TimestampMS & nConnectionFlags::TIME_MASK);
}

struct xSUA_ProducerService : xService {

    //
};

struct xSUA_ConsumerService : xService {

    xSUA_ConsumerService() {
        RuntimeAssert(NoConsumerPrinter.Init("NoConsumerPrinter"));
    }

    ~xSUA_ConsumerService() {
        NoConsumerPrinter.Clean();
    }

    void OnClientConnected(xServiceClientConnection & Connection) override {
        ChallengingServerList.push_back(Connection.GetConnectionId());
        auto & Flags = Connection.GetUserContext().U64;
        Flags        = nConnectionFlags::CHALLENGING;
    }

    void OnClientClose(xServiceClientConnection & Connection) override {
        auto & Flags = Connection.GetUserContext().U64;
        if (Flags & nConnectionFlags::CHALLENGING) {
            X_DEBUG_PRINTF("closing challengin connection");
            return;
        }

        //
        auto it = std::lower_bound(ServerList.begin(), ServerList.end(), &Connection);
        assert(it != ServerList.end() && *it == &Connection);
        ServerList.erase(it);

        DoCleanupConnection(Connection);
    }

    void SetConnectionChallenging(xServiceClientConnection & Connection) {
        Connection.GetUserContext().U64 |= nConnectionFlags::CHALLENGING;
    }

    void SetConnectionReady(xServiceClientConnection & Connection) {
        auto F = GetConnectionFlags(Connection);
        if (F != nConnectionFlags::CHALLENGING) {
        }
    }

    void DoInsertConnection(xServiceClientConnection & Connection) {
    }

    void DoCleanupConnection(xServiceClientConnection & Connection) {
        X_DEBUG_PRINTF("");
    }

    //
    void OnTick(uint64_t NowMS) override {
    }

private:
    xCollectableErrorPrinter NoConsumerPrinter;
};

xSUA_ProducerService PS;
xSUA_ConsumerService CS;

int main(int argc, char ** argv) {

    auto CL = GetConfigLoader(argc, argv);
    CL.Require(ProducerAddress, "ProducerAddress");
    CL.Require(ConsumerAddress, "ConsumerAddress");

    auto PSG = xResourceGuard(PS, &IC, ProducerAddress);
    auto CSG = xResourceGuard(CS, &IC, ProducerAddress);

    RuntimeAssert(PSG);
    RuntimeAssert(CSG);

    while (true) {
        GlobalTicker.Update();
        IC.LoopOnce();
        TickAll(GlobalTicker(), PS, CS);
    }

    return 0;
}
