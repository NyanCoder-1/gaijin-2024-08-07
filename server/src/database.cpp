#include "database.hpp"
#include "resources.hpp"
#include "statistics.hpp"
#include <atomic>
#include <condition_variable>
#include <fstream>
#include <mutex>
#include <shared_mutex>
#include <thread>
#include <unordered_map>
#include <nlohmann/json.hpp>

constexpr auto FILENAME = "config.txt";

namespace {
    struct Value {
        std::string value;
        Statistics::Counter statsCounter;
        Value() = default;
        Value(const std::string& value) : value(value) {}
        Value& operator=(const std::string& value) {
            this->value = value;
            return *this;
        }
        operator std::string() const {
            return value;
        }
    };
    std::unordered_map<std::string, Value> database;
    std::shared_mutex dataLockingMutex;
    std::atomic<bool> writeFlag = true;

    std::thread databaseSaveThread;
    std::condition_variable databaseSaveSignal;
    std::mutex databaseSaveMutex;
}

void Database::Load()
{
    std::ifstream fin(FILENAME);
    if (!fin.is_open())
        return;
    nlohmann::json data;
    fin >> data;
    fin.close();
    if (std::unique_lock databaseWriteLock(dataLockingMutex); databaseWriteLock)
    {
        for (auto& [key, value] : data.items())
            database[key] = (std::string)value;
    }
    writeFlag = false;
}

void Database::Save()
{
    if (!writeFlag)
        return;
    writeFlag = false;
    nlohmann::json data = nlohmann::json::object();
    if (std::shared_lock databaseReadLock(dataLockingMutex); databaseReadLock)
    {
        data = database;
    }
    std::ofstream fout(FILENAME);
    fout << data.dump();
    fout.close();
}

void Database::startPeriodicSave()
{
    std::thread periodicSave([]() {
        auto now = std::chrono::high_resolution_clock::now();
        while (!getExitFlag()) {
            std::unique_lock lock(databaseSaveMutex);
            databaseSaveSignal.wait_until(lock, now + std::chrono::seconds(DATABASE_SAVE_PERIOD));
            now = std::chrono::high_resolution_clock::now();

            Database::Save();
        }
    });
    databaseSaveThread.swap(periodicSave);
}

void Database::waitPeriodicSave()
{
    if (databaseSaveThread.joinable())
        databaseSaveThread.join();
}

void Database::interruptPeriodicSaveTimer()
{
    databaseSaveSignal.notify_all();
}

std::string Database::Get(const std::string &key)
{
    if (std::shared_lock databaseReadLock(dataLockingMutex); databaseReadLock)
    {
        Statistics::incrementGet();
        auto it = database.find(key);
        if (it != database.end())
        {
            it->second.statsCounter.incrementRead();
            return key + "=" + (std::string)it->second + "\n" + (std::string)it->second.statsCounter;
        }
        else {
            databaseReadLock.unlock();
            if (std::unique_lock databaseWriteLock(dataLockingMutex); databaseWriteLock) {
                auto &element = database[key];
                writeFlag = true;
                // after creating a new element unlock the write lock of the database to allow reading
                databaseWriteLock.unlock();
                databaseReadLock.lock();
                element.statsCounter.incrementRead();
                return key + "=" + (std::string)element + "\n" + (std::string)element.statsCounter;
            }
        }
    }
    return key + "=\n" + Statistics::Counter::Create().get()->toString();
}

std::string Database::Set(const std::string &key, const std::string &value)
{
    if (std::unique_lock databaseWriteLock(dataLockingMutex); databaseWriteLock)
    {
        Statistics::incrementSet();
        auto &element = database[key];
        element.value = value;
        writeFlag = true;
        // after write operations unlock the database to allow reading, and in either case the counter relies on std::atomic
        databaseWriteLock.unlock();
        element.statsCounter.incrementWrite();
        return key + "\n" + (std::string)element.statsCounter;
    }
    return key + "\n" + Statistics::Counter::Create().get()->toString();
}
