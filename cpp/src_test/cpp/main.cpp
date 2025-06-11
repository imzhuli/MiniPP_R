#include "../lib_server_util/command_line.hpp"

#include <iostream>
#include <string>
#include <type_traits>

//
using namespace std;

struct xX;
struct xY;

struct xX {
    xX() = default;
    xX(const xY & y) { cout << "x(y)" << endl; }
};

struct xY {
    operator xX() const {
        cout << "y->x" << endl;
        return xX();
    }
};

struct xZ {
    xZ(const xX &){};
};

struct xYY : xY {};

void foo(const xX &) {
}

int main(int argc, char ** argv) {

    auto E = xRuntimeEnv::FromCommandLine(argc, argv);
    cout << ToString(E) << endl;

    xY y;
    xZ z(y);

    xYY yy;
    xZ  zz(yy);

    return 0;
}
