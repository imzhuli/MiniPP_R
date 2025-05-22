#include "./_global.hpp"

#include <librdkafka/rdkafkacpp.h>

using namespace ::std::chrono_literals;

// security.protocol: SASL_PLAINTEXT
// sasl.mechanism: SCRAM-SHA-256
// username="client" password="client123456";

xODI_LocalAudit ODI_LocalAudit;
