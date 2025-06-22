#include "../lib_server_util/all.hpp"

#include <pp_common/base.hpp>

auto IC  = xIoContext();
auto ICG = xResourceGuard(IC);

int main(int argc, char ** argv) {
    auto SEG = xServiceEnvGuard(argc, argv);

    return 0;
}
