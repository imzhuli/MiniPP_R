#pragma once
#include <pp_common/_.hpp>

extern std::vector<std::string> KafkaServerAddressList;
extern std::string              KafkaTopic;
extern int                      KafdaPartition;

extern struct xODI_LocalAudit {

} ODI_LocalAudit;

extern bool InitKafka();
extern void CleanKafka();
