#pragma once
#include <pp_common/base.hpp>

static constexpr const size_t MAX_AUTH_BACKEND_SERVER_COUNT = 150;

std::vector<xNetAddress> LoadAuthBackendServerList(const char * filename);
