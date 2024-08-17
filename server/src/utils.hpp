#pragma once

#include <cstdint>
#include <functional>
#include <system_error>
#include <boost/system/error_category.hpp>

// this is from https://stackoverflow.com/q/27498592/5442026
constexpr std::size_t constLength(const char* str)
{
    return (*str == 0) ? 0 : constLength(str + 1) + 1;
}

void handleCtrlC(const std::function<void()> callback);

enum class ConnectionError : int {
    UnknownCommand = 1,
    InvalidSet
};
class ConnectionErrorCategory : public boost::system::error_category {
public:
    const char* name() const noexcept override;
    std::string message(int errorCode) const override;
};


