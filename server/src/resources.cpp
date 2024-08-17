#include "resources.hpp"
#include <atomic>

namespace {
    std::atomic<bool> exitFlag = false;
}

void setExitFlag(bool flag)
{
    exitFlag = flag;
}
bool getExitFlag()
{
    return exitFlag;
}