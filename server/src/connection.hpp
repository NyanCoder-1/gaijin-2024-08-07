#pragma once

#include <functional>
#include <vector>
#include <memory>
#include <boost/smart_ptr.hpp>
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
        bool processRequest(const std::vector<char>& requestData, std::function<void()> callback, std::function<void(const boost::system::error_code &eCode)> onError);
        void send(const std::string& responseData, std::function<void()> callback, std::function<void(const boost::system::error_code &eCode)> onError);

        void get(const std::string& key, std::function<void()> callback, std::function<void(const boost::system::error_code &eCode)> onError);
        void set(const std::string& key, const std::string& value, std::function<void()> callback, std::function<void(const boost::system::error_code &eCode)> onError);

        boost::asio::ip::tcp::socket sock;
        boost::asio::streambuf buf;
    };
}