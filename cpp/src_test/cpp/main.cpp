#include "../lib_server_util/command_line.hpp"

#include <iostream>
#include <string>
#include <type_traits>

//
using namespace std;

void test(auto && v) {
    cout << std::is_reference_v<decltype(v)> << endl;
    cout << std::is_rvalue_reference_v<decltype(v)> << endl;

    cout << std::is_reference_v<decltype((v))> << endl;
    cout << std::is_rvalue_reference_v<decltype((v))> << endl;
}

int main(int argc, char ** argv) {

    auto E = ParseEnv(argc, argv);
    cout << E.ToString() << endl;
    test(1);

    return 0;
}
