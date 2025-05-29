#pragma once

#include <pp_common/_.hpp>

class xCollectableErrorPrinter {
public:
    bool Init(const std::string & ErrorMessage, size_t TriggerCountLimit = 1'000, uint64_t TriggerTimeoutLimitMS = 60'000);
    bool Init(xLogger * LoggerPtr, const std::string & ErrorMessage, size_t TriggerCountLimit, uint64_t TriggerTimeoutLimitMS);
    void Clean();
    void Hit();

private:
    xLogger *   LoggerPtr;
    std::string ErrorMessage;
    size_t      Counter                = 0;
    size_t      TriggerCountLimit      = 0;
    uint64_t    LastTriggerTimestampMS = 0;
    uint64_t    TriggerTimeoutLimitMS  = 0;
};
