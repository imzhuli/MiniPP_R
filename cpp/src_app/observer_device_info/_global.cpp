#include "./_global.hpp"

#include <librdkafka/rdkafkacpp.h>

std::vector<std::string> KafkaServerAddressList;
std::string              KafkaTopicName = "my-topic";
int                      KafdaPartition = RdKafka::Topic::PARTITION_UA;

// security.protocol: SASL_PLAINTEXT
// sasl.mechanism: SCRAM-SHA-256
// username="client" password="client123456";

xODI_LocalAudit ODI_LocalAudit;

static RdKafka::Conf * KfkConf = nullptr;
class xKfkDeliveryReportCb : public RdKafka::DeliveryReportCb {
public:
    void dr_cb(RdKafka::Message & message) {
        cout << "CallbackThread: " << std::this_thread::get_id() << endl;
        if (message.err()) {
            std::cerr << "消息发送失败: " << message.errstr() << std::endl;
        } else {
            std::cout << "消息发送成功，偏移量: " << message.offset() << std::endl;
        }
    }
};
static xKfkDeliveryReportCb KfkCB;
static RdKafka::Producer *  KfkProducer = nullptr;

bool InitKafka() {

    KfkConf = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);
    RuntimeAssert(KfkConf);
    auto KC = xScopeGuard([&] { delete Steal(KfkConf); });

    std::string errstr;
    RuntimeAssert(RdKafka::Conf::CONF_OK == KfkConf->set("security.protocol", "SASL_PLAINTEXT", errstr));
    RuntimeAssert(RdKafka::Conf::CONF_OK == KfkConf->set("sasl.mechanism", "SCRAM-SHA-256", errstr));
    RuntimeAssert(RdKafka::Conf::CONF_OK == KfkConf->set("sasl.username", "client", errstr));
    RuntimeAssert(RdKafka::Conf::CONF_OK == KfkConf->set("sasl.password", "client123456", errstr));

    if (KfkConf->set("bootstrap.servers", "45.197.7.51:9085", errstr) != RdKafka::Conf::CONF_OK) {
        std::cerr << "设置 bootstrap.servers 失败: " << errstr << std::endl;
        return false;
    }

    KfkProducer = RdKafka::Producer::create(KfkConf, errstr);
    if (!KfkProducer) {
        std::cerr << "无法创建生产者: " << errstr << std::endl;
        return false;
    }
    auto KPG = xScopeGuard([&] { delete KfkProducer; });

    RdKafka::Topic * KafkaTopic = RdKafka::Topic::create(KfkProducer, KafkaTopicName, NULL, errstr);
    if (!KafkaTopic) {
        std::cerr << "无法创建主题对象: " << errstr << std::endl;
        return false;
    }

    cout << "StarterThread: " << std::this_thread::get_id() << endl;

    for (int i = 0; i < 10; ++i) {
        std::string payload = "Message " + std::to_string(i);
        // 发送消息（异步）
        RdKafka::ErrorCode resp = KfkProducer->produce(
            KafkaTopic,                          // 主题对象
            KafdaPartition,                      // 分区（PARTITION_UA 表示自动分配）
            RdKafka::Producer::RK_MSG_COPY,      // 消息复制策略
            const_cast<char *>(payload.data()),  // 消息内容
            payload.size(),                      // 消息长度
            NULL,                                // 键（可选）
            NULL                                 // 消息头（可选）
        );
        if (resp != RdKafka::ERR_NO_ERROR) {
            std::cerr << "发送消息失败: " << RdKafka::err2str(resp) << std::endl;
        } else {
            std::cout << "已发送消息: " << payload << std::endl;
        }
        // 7. 定期轮询以处理事件（如交付报告回调）
        KfkProducer->poll(0);
    }
    KfkProducer->flush(10000);
    KfkProducer->poll(0);

    return true;
}

void CleanKafka() {
}