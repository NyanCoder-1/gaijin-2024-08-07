#include "utils.hpp"
#include <csignal>

namespace {
    std::function<void()> interruptCallback;
    void interruptHandler(int s) {
        if ((s == SIGINT) && interruptCallback) {
            interruptCallback();
        }
    }
}

void handleCtrlC(const std::function<void()> callback)
{
    interruptCallback = callback;
    struct sigaction sigIntHandler;
    sigIntHandler.sa_handler = interruptHandler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;
    sigaction(SIGINT, &sigIntHandler, nullptr);
}

const char *ConnectionErrorCategory::name() const noexcept
{
    return "Connection Error";
}

std::string ConnectionErrorCategory::message(int errorCode) const
{
    switch (static_cast<ConnectionError>(errorCode)) {
    case ConnectionError::UnknownCommand:
        return "Unknown command";
    case ConnectionError::InvalidSet:
        return "Invalid set command";
    }
    return "Unknown error";
}
