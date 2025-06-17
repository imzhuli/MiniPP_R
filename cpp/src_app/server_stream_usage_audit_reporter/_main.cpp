#include "../lib_server_util/all.hpp"

#include <pp_common/_.hpp>

int main(int, char **) {

    std::string input = "  [ '45.197.7.51:9085', '33mm\\'1:9085']  ";

    auto P = ParsePythonStringArray(input);
    if (!P()) {
        cerr << "Failed to parse python string array" << endl;
        return -1;
    }

    for (auto & I : *P) {
        cout << ">> " << I << endl;
    }
    return 0;
}
