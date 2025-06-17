#include <pp_common/base.hpp>

auto IC  = xIoContext();
auto ICG = xResourceGuard(IC);

int main(int argc, char ** argv) {

    auto CL = xCommandLine(
        argc, argv,
        {
            { 'c', "config", "config", true },
        }
    );

    return 0;
}
