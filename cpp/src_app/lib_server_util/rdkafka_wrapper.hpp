#pragma once
#include <librdkafka/rdkafkacpp.h>

#include <map>
#include <pp_common/_.hpp>

class xKfkProducer final {
public:
    bool Init(const std::string & Topic, const std::map<std::string, std::string> & KafkaParams);
    void Clean();

    bool Post(const std::string & Key, const void * DataPtr, const size_t Size);
    bool Post(const std::string & Key, std::string_view Data) {
        return Post(Key, Data.data(), Data.size());
    }
    bool Post(std::string_view Data) {
        return Post({}, Data.data(), Data.size());
    }

    void Flush();
    void Poll();

protected:
    bool CreateProducer();
    void DestroyProducer();
    void CheckAndRecreateProducer();

    // run in poll thread:

private:
    RdKafka::Conf * KfkConf = nullptr;
    std::string     KfkToipcName;

    RdKafka::Producer * KfkProducer = nullptr;
    RdKafka::Topic *    KfkTopic    = nullptr;
};
