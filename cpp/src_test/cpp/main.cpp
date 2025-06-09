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

int main(int, char **) {

    test(1);

    return 0;
}
