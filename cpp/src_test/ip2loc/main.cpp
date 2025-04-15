#include <pp_common/ipdb/Ipdb.hpp>

int main(int, char **) {
    auto filename = "./test_assets/ip-loc.BIN";

    auto ipdb = xIpDb();
    ipdb.Init(filename);

    auto a = ipdb.Get("240e:ff:e020:98c:0:ff:b061:c306");
    if (!a()) {
        cerr << "ip not found" << endl;
    } else {
        auto & r = *a;
        cout << "country code: " << HexShow(&r.CountryCode, sizeof(r.CountryCode)) << endl;
        cout << "city code: " << r.CityCode << endl;
    }
    return 0;
}
