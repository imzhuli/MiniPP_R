#include <string>
//
using namespace std;

struct Test {

    Test(const std::string &) {}
    ~Test() = default;
};

int main(int, char **) {

    Test t = { "hello" };

    return 0;
}
