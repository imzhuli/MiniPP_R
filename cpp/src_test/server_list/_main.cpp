#include <pp_component/server_list/server_list.hpp>

int main(int, char **) {

    xServerInfoManager SIM;
    RuntimeAssert(SIM.Init());

    xNetAddress Addr1 = xNetAddress::Parse("192.168.10.1:8888");
    xNetAddress Addr2 = xNetAddress::Parse("192.168.10.2:8888");
    xNetAddress Addr3 = xNetAddress::Parse("192.168.10.3:8888");
    xNetAddress Addr4 = xNetAddress::Parse("192.168.10.4:8888");
    xNetAddress Addr5 = xNetAddress::Parse("192.168.10.5:8888");

    cout << Addr1.ToString() << endl;

    SIM.AddServerInfo(Addr1);
    SIM.AddServerInfo(Addr2);
    SIM.AddServerInfo(Addr3);
    SIM.AddServerInfo(Addr4);
    SIM.AddServerInfo(Addr5);

    SIM.RemoveServerByIndex(0);
    SIM.RemoveServerByIndex(2);
    SIM.RemoveServerByIndex(4);

    SIM.AddServerInfo(Addr1);

    cout << SIM.Dump() << endl;

    SIM.Clean();
    return 0;
    //
}
