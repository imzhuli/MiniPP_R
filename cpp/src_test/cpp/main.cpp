#include "../lib_server_util/command_line.hpp"

#include <iostream>
#include <string>
#include <type_traits>

//
using namespace std;

// FUCK gcc: the following will cause -Werror=stringopo-overflow=
// void D(size_t DataSize) {  // cause gcc bug:
//     if (!DataSize || DataSize % 16) {
//         return;
//     }
//     auto R       = std::vector<ubyte>(DataSize);
//     auto padding = (size_t)(R[DataSize - 1]);
//     if (padding > 16) {
//         return;
//     }
//     R.resize(DataSize - padding);
//     return;
// }

int main(int argc, char ** argv) {

    auto CL = xCommandLine{ argc, argv, { { 'l', nullptr, "loop", false } } };

    if (CL["loop"]) {
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(60));
        }
    }

    return 0;
}
