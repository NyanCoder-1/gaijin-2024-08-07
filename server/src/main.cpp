#include "connection.hpp"
#include "database.hpp"
#include "resources.hpp"
#include "statistics.hpp"
#include <atomic>
#include <chrono>
#include <iostream>
#include <thread>
#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>

//
// the server connection architecture was inspired from
// https://www.codeproject.com/Articles/1264257/Socket-Programming-in-Cplusplus-using-boost-asio-T
// and https://www.codingwiththomas.com/blog/boost-asio-server-client-example
//

namespace {
    void accept();
    void handle_accept(Connection::Handler::Ptr handler, const boost::system::error_code& ec);
    boost::asio::io_service ioService;
    boost::asio::ip::tcp::acceptor acceptor(ioService, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), PORT));

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
}

int main()
{
    try {
        handleCtrlC([]() {
            ioService.stop();
            setExitFlag(true);
            Database::interruptPeriodicSaveTimer();
        });
        Database::Load();
        accept();
        Statistics::startPeriodicPrint();
        Database::startPeriodicSave();
        ioService.run();
        Database::waitPeriodicSave();
    }
    catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}