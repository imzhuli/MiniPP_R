#include <pp_common/request_id_pool/request_id_pool.hpp>
#include <thread>

int main(int, char **) {

    auto Pool = xPPC_RequestIdPool();
    Pool.Init(1024);

    auto RID = Pool.Acquire({ 123, 456 });
    X_DEBUG_PRINTF("Request: %" PRIx64 "", RID);

    auto RID_Expire = Pool.Acquire({ 777, 888 });
    X_DEBUG_PRINTF("Request: %" PRIx64 "", RID_Expire);

    auto S = xPPC_RequestSource();
    if (!Pool.CheckAndRelease(RID, S)) {
        cout << "error" << endl;
        return -1;
    }

    Pool.Tick();
    std::this_thread::sleep_for(std::chrono::seconds(2));
    Pool.Tick();

    if (Pool.CheckAndRelease(RID_Expire, S)) {
        cout << "error on expired pair" << endl;
        return -1;
    }

    auto Found = Pool.CheckAndRelease(123123213, X2R(xPPC_RequestSource()));
    cout << "Found ?= " << YN(Found) << endl;

    // test pool cleaner
    auto RID_Ignored = Pool.Acquire({ 1024, 1024 });
    Touch(RID_Ignored);

    Pool.Clean();
    return 0;
};
