#pragma once

#include <cstddef>
#include <atomic>
#include <memory>
#include <string>

namespace Statistics {
    typedef struct {
        std::size_t get = 0;
        std::size_t set = 0;
    } Requests;
    const Requests getRequestsAmount();
    void incrementGet();
    void incrementSet();

    // this class is for counting reads and writes for each key
    class Counter {
    public:
        static std::shared_ptr<Statistics::Counter> Create();

        Counter() = default;
        ~Counter() = default;
        Counter(const Counter&) = delete;
        Counter& operator=(const Counter&) = delete;
        Counter(Counter&&) = delete;
        Counter& operator=(Counter&&) = delete;

        void incrementRead();
        void incrementWrite();

        std::size_t getRead() const;
        std::size_t getWrite() const;

        std::string toString() const;
        operator std::string() const {
            return this->toString();
        }

    private:
        std::atomic<std::size_t> read = 0;
        std::atomic<std::size_t> write = 0;
    };
}
