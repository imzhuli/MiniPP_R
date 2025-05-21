#include "./rdkafka_wrapper.hpp"

#include <typeinfo>

/*
TODO
    在大量发送消息后. 进行重置producer, 这里要考虑线程同步. 以及回收旧对象前的flush和超时操作

*/

/* Callback object */

class xKfkDeliveryReportCb : public RdKafka::DeliveryReportCb {
public:
    void dr_cb(RdKafka::Message & message) {
        cout << "CallbackThread: " << std::this_thread::get_id() << endl;
        if (message.err()) {
            std::cerr << "消息发送失败: " << message.errstr() << std::endl;
        } else {
            std::cout << "消息发送成功，偏移量: " << message.offset() << ", 延迟: " << message.latency() << std::endl;
        }
    }
};
static xKfkDeliveryReportCb KfkCB;

/**/

bool xKfkProducer::Init(const std::string & Topic, const std::map<std::string, std::string> & KafkaParams) {
    KfkConf = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);
    if (!KfkConf) {
        X_DEBUG_PRINTF("Failed to create kafka config");
        return false;
    }
    auto ConfCleaner = xScopeGuard([this] { delete Steal(KfkConf); });

    KfkToipcName          = Topic;
    auto TopicNameCleaner = xScopeGuard([this] { Reset(KfkToipcName); });

    std::string errstr;
    for (auto & [K, V] : KafkaParams) {
        if (RdKafka::Conf::CONF_OK != KfkConf->set(K, V, errstr)) {
            X_DEBUG_PRINTF("failed to set kafka producer param: %s -> %s, error=%s", K.c_str(), V.c_str(), errstr.c_str());
            return false;
        }
    }
    if (RdKafka::Conf::CONF_OK != KfkConf->set("dr_cb", &KfkCB, errstr)) {
        X_DEBUG_PRINTF("failed to set kafka producer param: %s, error=%s", "dr_cb", errstr.c_str());
        return false;
    }

    auto FirstCreate = CreateProducer();
    if (!FirstCreate) {
        X_DEBUG_PRINTF("Failed to create producer with topic");
        return false;
    }

    ConfCleaner.Dismiss();
    TopicNameCleaner.Dismiss();

    X_DEBUG_PRINTF("done");
    return true;
}

void xKfkProducer::Clean() {
    if (KfkProducer) {
        assert(KfkTopic);
        DestroyProducer();
    }
    auto ConfCleaner      = xScopeGuard([this] { delete Steal(KfkConf); });
    auto TopicNameCleaner = xScopeGuard([this] { Reset(KfkToipcName); });
    X_DEBUG_PRINTF("done");
}

bool xKfkProducer::CreateProducer() {
    assert(!KfkProducer);
    std::string errstr;
    KfkProducer = RdKafka::Producer::create(KfkConf, errstr);
    if (!KfkProducer) {
        X_DEBUG_PRINTF("error=%s", errstr.c_str());
        return false;
    }
    auto KPG = xScopeGuard([this] { delete Steal(KfkProducer); });

    KfkTopic = RdKafka::Topic::create(KfkProducer, KfkToipcName, NULL, errstr);
    if (!KfkTopic) {
        X_DEBUG_PRINTF("error=%s", errstr.c_str());
        return false;
    }
    auto KTG = xScopeGuard([this] { delete Steal(KfkTopic); });

    KPG.Dismiss();
    KTG.Dismiss();
    return true;
}

void xKfkProducer::DestroyProducer() {
    assert(KfkProducer && KfkTopic);
    auto KPG = xScopeGuard([this] { delete Steal(KfkProducer); });
    auto KTG = xScopeGuard([this] { delete Steal(KfkTopic); });
}

void xKfkProducer::CheckAndRecreateProducer() {
    Pure();
}

bool xKfkProducer::Post(const std::string & Key, const void * DataPtr, const size_t Size) {
    RdKafka::ErrorCode resp = KfkProducer->produce(
        KfkTopic,                        // 主题对象
        RdKafka::Topic::PARTITION_UA,    // 分区（PARTITION_UA 表示自动分配）
        RdKafka::Producer::RK_MSG_COPY,  // 消息复制策略
        (char *)(DataPtr),               // 消息内容
        Size,                            // 消息长度
        (Key.empty() ? nullptr : &Key),  // 键（可选）
        NULL                             // 消息头（可选）
    );
    if (resp != RdKafka::ERR_NO_ERROR) {
        X_DEBUG_PRINTF("failed to post payload");
        return false;
    }
    return true;
}

void xKfkProducer::Flush() {
    if (!KfkProducer) {
        return;
    }
    KfkProducer->flush(0);
}

void xKfkProducer::Poll() {
    if (!KfkProducer) {
        return;
    }
    KfkProducer->poll(0);
}
