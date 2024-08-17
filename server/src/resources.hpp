#pragma once

#include "utils.hpp"
#include <cstdint>

// packet config
constexpr auto MAX_LENGTH = 1024;
constexpr auto MIN_LENGTH = 16;
constexpr auto PACKET_ALIGNMENT = 4;
typedef std::uint32_t pkgsize_t;
constexpr auto PACKET_HEADER_SIZE = sizeof(pkgsize_t);

// requests helper data
constexpr auto COMMAND_GET_START = "$get ";
constexpr auto COMMAND_GET_LENGTH = constLength(COMMAND_GET_START);
constexpr auto COMMAND_SET_START = "$set ";
constexpr auto COMMAND_SET_LENGTH = constLength(COMMAND_GET_START);

// periodic tasks config
constexpr auto STATISTICS_PRINT_PERIOD = 5;
constexpr auto DATABASE_SAVE_PERIOD = 2;

// server config
constexpr auto PORT = 8000;

void setExitFlag(bool flag);
bool getExitFlag();
