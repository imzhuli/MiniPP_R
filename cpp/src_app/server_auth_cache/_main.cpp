#include "../lib_backend_connection/backend_connection.hpp"

auto IC  = xIoContext();
auto ICG = xResourceGuard(IC);

auto TA = xNetAddress::Parse("45.202.204.29:20005");

int main(int, char **) {

    auto C  = xBackendConnection();
    auto CG = xResourceGuard(C, &IC, TA, "apitest", "123456");

    while (true) {
        IC.LoopOnce();
        C.Tick();
    }

    return 0;
    //
}
