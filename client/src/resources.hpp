#pragma once

#include <array>
#include <cstdint>
#include <random>

// server address
inline constexpr auto HOST = "127.0.0.1";
inline constexpr auto PORT = 8000;

// buffer config
inline constexpr auto MAX_LENGTH = 1024;
inline constexpr auto MIN_LENGTH = 16;
inline constexpr auto PACKET_ALIGNMENT = 4;
typedef std::uint32_t pkgsize_t;
inline constexpr auto PACKET_HEADER_SIZE = sizeof(pkgsize_t);

// randomizer
extern std::random_device dev;
extern std::mt19937 rng;

// requests helper data
inline constexpr auto REQUEST_GET = "$get ";
inline constexpr auto REQUEST_SET = "$set ";
extern std::uniform_int_distribution<std::mt19937::result_type> distProbability;
inline constexpr auto KEYS_AMOUNT = 10;
extern const std::array<std::string, KEYS_AMOUNT> keys;
extern std::uniform_int_distribution<std::mt19937::result_type> distKey;
extern const std::array<char, 62> charsCollection;
extern std::uniform_int_distribution<std::mt19937::result_type> distChar;
extern std::uniform_int_distribution<std::mt19937::result_type> distValueLength;

inline constexpr auto REQUESTS_AMOUNT = 10000;