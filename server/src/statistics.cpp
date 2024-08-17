#include "statistics.hpp"
#include "resources.hpp"
#include <atomic>
#include <iostream>
#include <thread>

namespace {
    std::atomic<std::size_t> requestsGet = 0;
    std::atomic<std::size_t> requestsSet = 0;
}

const Statistics::Requests getRequestsAmount()
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

void Statistics::startPeriodicPrint()
{
    std::thread t([](){
        while (!getExitFlag()) {
            auto now = std::chrono::high_resolution_clock::now();
            auto stats = getRequestsAmount();
            std::cout << "Statistics of previous 5 seconds:" << std::endl << "\tget=" << stats.get << std::endl << "\tset=" << stats.set << std::endl;
            std::this_thread::sleep_until(now + std::chrono::seconds(STATISTICS_PRINT_PERIOD));
        }
    });
    t.detach();
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
