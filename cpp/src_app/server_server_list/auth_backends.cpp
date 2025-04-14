#include "./auth_backends.hpp"

#include <core/string.hpp>

std::vector<xNetAddress> LoadAuthBackendServerList(const char * filename) {
    auto S     = std::vector<xNetAddress>();
    auto Lines = FileToLines(filename);
    for (auto & L : Lines) {
        auto T = Trim(L);
        if (T.empty() || T[0] == '#') {
            continue;
        }
        auto A = xNetAddress::Parse(T);
        if (!A || !A.Port) {
            cerr << "invalid address: " << T << endl;
            return {};
        }
        X_DEBUG_PRINTF("Add auth backend server: %s", A.ToString().c_str());
        S.push_back(A);
    }
    return S;
}
