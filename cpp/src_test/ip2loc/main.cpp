#include <pp_common/ipdb/Ipdb.hpp>

int main(int argc, char ** argv) {
    auto CL = xCommandLine(
        argc, argv,
        {
            { 'n', "filenmae", "filename", true },
        }
    );

    auto RegionFileName = "./test_assets/ip-loc-region-list.csv";
    auto filename       = "./test_assets/ip-loc.BIN";

    auto Opt = CL["filename"];
    if (Opt) {
        filename = Opt->c_str();
    }

    auto ipdb = xIpDb();
    ipdb.Init(RegionFileName, filename);

    // auto a = ipdb.Get("240e:ff:e020:98c:0:ff:b061:c306");
    auto a = ipdb.Get("183.2.172.177");
    if (!a) {
        cerr << "ip not found" << endl;
    } else {
        auto & r = *a;
        cout << "country code: " << HexShow(&r.CountryId, sizeof(r.CountryId)) << endl;
        cout << "city code: " << r.CityId << endl;
    }
    return 0;
}
