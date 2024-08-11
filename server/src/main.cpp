#include "connection.hpp"
#include "database.hpp"
#include "statistics.hpp"
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <csignal>
#include <iostream>
#include <thread>
#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>

constexpr auto PORT = 8000;

//
// the server connection architecture was inspired from
// https://www.codeproject.com/Articles/1264257/Socket-Programming-in-Cplusplus-using-boost-asio-T
// and https://www.codingwiththomas.com/blog/boost-asio-server-client-example
//

namespace {
    void accept();
    void handle_accept(Connection::Handler::Ptr handler, const boost::system::error_code& ec);
    void periodicPrintStatistics();
    boost::asio::io_service ioService;
    boost::asio::ip::tcp::acceptor acceptor(ioService, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), PORT));
    std::thread databaseSaveThread([](){});
    std::condition_variable databaseSaveSignal;
    std::mutex databaseSaveMutex;
    std::atomic<bool> exitFlag = false;
    constexpr auto STATISTICS_PRINT_PERIOD = 5;
    constexpr auto DATABASE_SAVE_PERIOD = 2;

    void accept()
    {
        auto connection = Connection::Handler::Create((boost::asio::io_context&)acceptor.get_executor().context());

        acceptor.async_accept(connection->getSocket(),
            boost::bind(&handle_accept, connection, boost::asio::placeholders::error));
    }
    void handle_accept(Connection::Handler::Ptr handler, const boost::system::error_code& ec)
    {
        if (!ec) {
            handler->process();
        }
        accept();
    }
    void periodicPrintStatistics()
    {
        while (!exitFlag) {
            auto now = std::chrono::high_resolution_clock::now();
            auto stats = Statistics::getRequestsAmount();
            std::cout << "Statistics of previous 5 seconds:" << std::endl << "\tget=" << stats.get << std::endl << "\tset=" << stats.set << std::endl;
            std::this_thread::sleep_until(now + std::chrono::seconds(STATISTICS_PRINT_PERIOD));
        }
    }
    void periodicDatabaseSave()
    {
        auto now = std::chrono::high_resolution_clock::now();
        while (!exitFlag) {
            std::unique_lock lock(databaseSaveMutex);
            databaseSaveSignal.wait_until(lock, now + std::chrono::seconds(DATABASE_SAVE_PERIOD));
            now = std::chrono::high_resolution_clock::now();

            Database::Save();
        }
    }
    void interruptHandler(int s)
    {
        if (s == SIGINT) {
            ioService.stop();
            exitFlag = true;
            databaseSaveSignal.notify_all();
        }
    }
}

int main()
{
    try {
        { // ^C handler
            struct sigaction sigIntHandler;
            sigIntHandler.sa_handler = interruptHandler;
            sigemptyset(&sigIntHandler.sa_mask);
            sigIntHandler.sa_flags = 0;
            sigaction(SIGINT, &sigIntHandler, nullptr);
        }
        Database::Load();
        accept();
        std::thread statisticsPrintThread(periodicPrintStatistics);
        statisticsPrintThread.detach();
        std::thread databaseSaveThread(periodicDatabaseSave);
        ioService.run();
        databaseSaveThread.join();
    }
    catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}