#include "statistics.hpp"
#include <atomic>

namespace {
    std::atomic<std::size_t> requestsGet = 0;
    std::atomic<std::size_t> requestsSet = 0;
}

const Statistics::Requests Statistics::getRequestsAmount()
{
    return Statistics::Requests{ .get = requestsGet.exchange(0), .set = requestsSet.exchange(0) };
}
void Statistics::incrementGet()
{
    requestsGet++;
}
void Statistics::incrementSet()
{
    requestsSet++;
}

std::shared_ptr<Statistics::Counter> Statistics::Counter::Create()
{
    return std::shared_ptr<Statistics::Counter>(new Statistics::Counter());
}

void Statistics::Counter::incrementRead()
{
    this->read++;
}

void Statistics::Counter::incrementWrite()
{
    this->write++;
}

std::size_t Statistics::Counter::getRead() const
{
    return this->read;
}
std::size_t Statistics::Counter::getWrite() const
{
    return this->write;
}

std::string Statistics::Counter::toString() const
{
    auto reads = this->getRead();
    auto writes = this->getWrite();
    return "reads=" + std::to_string(reads) + "\nwrites=" + std::to_string(writes);
}
