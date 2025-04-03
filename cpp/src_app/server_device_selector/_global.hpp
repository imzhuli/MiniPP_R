#pragma once
#include <pp_common/base.hpp>

extern xNetAddress              BindAddress;
extern xNetAddress              ReportDispatcherAddress;
extern std::vector<xNetAddress> RequestDispatcherAddress;

extern void LoadConfig(const char * filename);
