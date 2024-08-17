#pragma once

#include <functional>
#include <memory>
#include <string>
#include <tuple>
#include <vector>
#include <boost/asio.hpp>

namespace Connection {
    class Handler : public std::enable_shared_from_this<Connection::Handler> {
    struct Private{ explicit Private() = default; };
    public:
        typedef std::shared_ptr<Handler> Ptr;

        static Connection::Handler::Ptr Create(boost::asio::io_service& ioService) {
            return std::make_shared<Connection::Handler>(ioService, Private());
        }

        Handler(boost::asio::io_service& ioService, Private);
        Handler() = delete;
        Handler(const Handler&) = delete;
        Handler& operator=(const Handler&) = delete;
        Handler(Handler&&) = delete;
        Handler& operator=(Handler&&) = delete;
        virtual ~Handler() = default;

        boost::asio::ip::tcp::socket& getSocket();
        void process();

    private:
        const boost::system::error_code processRequest(const std::vector<char>& requestData);
        const std::tuple<const std::vector<char>, boost::system::error_code> receive();
        const boost::system::error_code send(const std::string &data);
        const boost::system::error_code send(const std::vector<char> &data);

        const boost::system::error_code get(const std::string& key);
        const boost::system::error_code set(const std::string& key, const std::string& value);

        boost::asio::ip::tcp::socket sock;
        boost::asio::streambuf buf;
    };
}