#pragma once

#include <memory>
#include <string>
#include <tuple>
#include <vector>
#include <boost/asio.hpp>

class Connection {
    struct Private{ explicit Private() = default; };
public:
    typedef std::shared_ptr<Connection> Ptr;

    Connection() = delete;
    Connection(Private);

    static Connection::Ptr Create() {
        return std::make_shared<Connection>(Private());
    }
    static constexpr bool IsGood(const boost::system::error_code &eCode, const bool excludeEof = false) {
        return !eCode || ((eCode == boost::asio::error::eof) && excludeEof);
    }

    const boost::system::error_code connect(const std::string& host, boost::asio::ip::port_type port);
    const bool isConnected() const;

    const std::tuple<const std::vector<char>, const boost::system::error_code> receive();
    const boost::system::error_code send(const std::vector<char>& data);
    const boost::system::error_code send(const std::string& data);

private:
    boost::asio::io_service ioService;
    boost::asio::ip::tcp::socket sock;
};
