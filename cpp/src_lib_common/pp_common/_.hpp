#pragma once
#include <core/core_min.hpp>
#include <core/core_os.hpp>
#include <core/core_stream.hpp>
#include <core/core_time.hpp>
#include <core/executable.hpp>
#include <core/indexed_storage.hpp>
#include <core/list.hpp>
#include <core/logger.hpp>
#include <core/string.hpp>
#include <crypto/base64.hpp>
#include <crypto/md5.hpp>
#include <network/net_address.hpp>
#include <network/tcp_connection.hpp>
#include <network/tcp_server.hpp>
#include <network/udp_channel.hpp>
#include <object/object.hpp>
#include <server_arch/client_pool.hpp>
#include <server_arch/message.hpp>
#include <server_arch/service.hpp>

//
#include <cinttypes>

//
using namespace xel::common;
using namespace std::chrono_literals;

// consts
using xel::InvalidDataSize;
using xel::MaxPacketPayloadSize;
using xel::MaxPacketSize;
using xel::PacketHeaderSize;

// type-defs
using xel::eLogLevel;
using xel::xAbstract;
using xel::xBaseLogger;
using xel::xBinaryMessage;
using xel::xClientPool;
using xel::xCommandLine;
using xel::xIndexedStorage;
using xel::xIndexId;
using xel::xIndexIdPool;
using xel::xIoContext;
using xel::xList;
using xel::xListNode;
using xel::xLogger;
using xel::xMd5Result;
using xel::xNetAddress;
using xel::xObjectIdManager;
using xel::xObjectIdManagerMini;
using xel::xPacket;
using xel::xPacketCommandId;
using xel::xPacketHeader;
using xel::xPacketRequestId;
using xel::xResourceGuard;
using xel::xScopeCleaner;
using xel::xScopeGuard;
using xel::xService;
using xel::xSocket;
using xel::xStreamReader;
using xel::xStreamWriter;
using xel::xTcpConnection;
using xel::xTcpServer;
using xel::xTicker;
using xel::xUdpChannel;

// functions
using xel::Base64Decode;
using xel::Base64Encode;
using xel::BuildPacket;
using xel::Daemonize;
using xel::GetTimestampMS;
using xel::HexShow;
using xel::HexToStr;
using xel::Md5;
using xel::Pure;
using xel::RuntimeAssert;
using xel::Split;
using xel::Steal;
using xel::StrToHex;
using xel::Todo;
using xel::Unreachable;
using xel::WriteMessage;
using xel::ZeroFill;

// std-lib:
#include <iostream>
using std::cerr;
using std::cout;
using std::endl;
using std::flush;

// min_defs:
using xVersion = uint32_t;

using xGroupId      = uint32_t;
using xServerId     = uint32_t;
using xAccountId    = uint64_t;
using xTerminalId   = uint64_t;
using xConnectionId = uint64_t;

using xContinentId = uint32_t;
using xCountryId   = uint32_t;
using xStateId     = uint32_t;
using xCityId      = uint32_t;

using xServerRouteId = uint16_t;  // 强行划分的服务器路径归属 id
using xSourcePoolId  = uint16_t;  //
using xAuditId       = uint64_t;

using xServerRuntimeKey = uint64_t;  // 由配置中心分配的动态服务器ID, 由序列和随机数组成

// min static constants
static constexpr const xSourcePoolId UNSPEC_SOURCE_POOL_ID = 0;
static constexpr const xSourcePoolId MAX_SOURCE_POOL_ID    = 255;  // included

static constexpr const size_t   DEFAULT_MAX_SERVER_CONNECTIONS    = 4096;
static constexpr const uint32_t MAX_RELAY_DEVICE_SERVER_SUPPORTED = 4096;  // IdManagerMini::MaxObjectId

enum eServerType : uint32_t {
    UNSPECIFIED = 0x00,

    // main service
    CONFIG_CENTER         = 0x01,  // CC
    DEVICE_RELAY          = 0x02,  // DR
    DEVICE_SELECTOR       = 0x03,  // DS
    THIRD_RELAY           = 0x04,  // TR
    STATIC_RELAY_TERMINAL = 0x05,  // SRT
    PROXY_ACCESS          = 0x06,  // PA
    ACCOUNT_CACHE         = 0x07,  // ACC
    AUDIT_CACHE           = 0x08,  // ADC

    // dispatchers
    CONFIG_CENTER_ENTRY   = 0x01'01,  // not used // 2025-04-07
    DEVICE_SELECTOR_ENTRY = 0x01'02,
    ACCOUNT_CACHE_ENTRY   = 0x01'03,
    AUDIT_CACHE_ENTRY     = 0x01'04,
};

static constexpr xContinentId CID_UNSPECIFIC    = 0x00;
static constexpr xContinentId CID_ASIA          = 0x01;
static constexpr xContinentId CID_EUROP         = 0x02;
static constexpr xContinentId CID_NORTH_AMERICA = 0x03;
static constexpr xContinentId CID_SOUTH_AMERICA = 0x04;
static constexpr xContinentId CID_AFRICA        = 0x05;
static constexpr xContinentId CID_OCEANIA       = 0x06;
static constexpr xContinentId CID_ANTARCTIC     = 0x07;

// clang-format off

static inline uint32_t High32(uint64_t U) { return (uint32_t)(U >> 32); }
static inline uint32_t Low32(uint64_t U)  { return (uint32_t)(U); }
static inline uint64_t Make64(uint32_t H32, uint32_t L32) { return (static_cast<uint64_t>(H32) << 32) + L32; }

static inline uint16_t High16(uint64_t U) { return (uint16_t)(U >> 48); }
static inline uint64_t Low48(uint64_t U)  { return U & 0x0000'FFFF'FFFF'FFFFu; }
static inline uint64_t Make64_H16L48(uint16_t H16, uint64_t L48) { return (static_cast<uint64_t>(H16) << 48) + L48; }

// clang-format on
