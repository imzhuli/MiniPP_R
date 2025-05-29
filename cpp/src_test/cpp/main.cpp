#include <pp_common/_.hpp>
//
using namespace std;
using namespace xel;

int main(int, char **) {

    auto M = static_cast<uint64_t>(xObjectIdManager::MaxObjectId);
    cout << M << endl;

    cout << hex;
    constexpr const auto Mask = (uint64_t(1) << 44) - 1;
    cout << Mask << endl;

    cout << ((M << 44) | Mask) << endl;

    return 0;
}
