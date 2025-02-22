#include <core/executable.hpp>
#include <pp_protocol/config_center_root/config_center_list.hpp>

using namespace xel;
using namespace std;

auto IC = xIoContext();
auto US = xUdpChannel();

struct xCCR_Service : public xUdpChannel::iListener {

    void OnData(xUdpChannel * ChannelPtr, ubyte * DataPtr, size_t DataSize, const xNetAddress & RemoteAddress) override {

    };

    //
};

static std::vector<xNetAddress> CCAddresses;
static xNetAddress              BindAddress;
static xCCR_Service             Service;

int main(int argc, char ** argv) {

    auto CL = xCommandLine(
        argc, argv,
        {
            { 'c', "config", "config", true },
            { 'b', "bind-address", "bind-address", true },
        }
    );

    auto OptB = CL["bind-address"];
    if (!OptB()) {
        cerr << "missing bind address" << endl;
        return -1;
    }

    BindAddress = xNetAddress::Parse(*OptB);
    if (!BindAddress || !BindAddress.Port) {
        cerr << "invalid bind address" << endl;
        return -1;
    }

    auto OptC = CL["config"];
    if (!OptC()) {
        cerr << "missing config file" << endl;
        return -1;
    }

    auto Lines = FileToLines(*OptC);
    if (Lines.empty()) {
        cerr << "no config center address found" << endl;
        return -1;
    }
    for (auto & L : Lines) {
        if (L.empty()) {
            cout << "empty line" << endl;
            continue;
        }
        auto A = xNetAddress::Parse(Trim(L));
        if (!A) {
            cerr << "invalid address" << endl;
            return -1;
        }
        CCAddresses.push_back(A);
    }

    auto ICG = xResourceGuard(IC);
    auto SC  = xUdpChannel();
    auto SCG = xResourceGuard(SC, &IC, BindAddress, &Service);
    if (!SCG) {
        cerr << "failed to bind server address" << endl;
        return -1;
    }

    while (true) {
        IC.LoopOnce();
    }

    return 0;
}
