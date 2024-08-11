#include "connection.hpp"
#include "database.hpp"
#include <arpa/inet.h>
#include <cstdint>
#include <iostream>
#include <sstream>
#include <boost/bind/bind.hpp>

constexpr auto MAX_LENGTH = 1024;
constexpr auto MIN_LENGTH = 16;
constexpr auto PACKET_ALIGNMENT = 4;
typedef std::uint32_t pkgsize_t;
constexpr auto PACKET_HEADER_SIZE = sizeof(pkgsize_t);

namespace {
    // this is from https://stackoverflow.com/q/27498592/5442026
    constexpr std::size_t constLength(const char* str)
    {
        return (*str == 0) ? 0 : constLength(str + 1) + 1;
    }
}

Connection::Handler::Handler(boost::asio::io_service& ioService, Connection::Handler::Private) :
    sock(ioService)
{}

bool Connection::Handler::processRequest(const std::vector<char> &requestData, std::function<void()> callback, std::function<void(const boost::system::error_code &eCode)> onError)
{
    constexpr auto COMMAND_GET_START = "$get ";
    constexpr auto COMMAND_GET_LENGTH = constLength(COMMAND_GET_START);
    constexpr auto COMMAND_SET_START = "$set ";
    constexpr auto COMMAND_SET_LENGTH = constLength(COMMAND_GET_START);

    if ((requestData.size() > COMMAND_GET_LENGTH) && !strncmp(requestData.data(), COMMAND_GET_START, COMMAND_GET_LENGTH)) {
        get(std::string(requestData.data() + COMMAND_GET_LENGTH, requestData.size() - COMMAND_GET_LENGTH).c_str(), callback, onError);
        return true;
    }
    else if ((requestData.size() > COMMAND_SET_LENGTH) && !strncmp(requestData.data(), COMMAND_SET_START, COMMAND_SET_LENGTH)) {
        auto body = std::string(requestData.data() + COMMAND_SET_LENGTH, requestData.size() - COMMAND_SET_LENGTH);
        auto pos = body.find('=');
        if (pos != std::string::npos) {
            const std::string key = body.substr(0, pos).c_str();
            const std::string value = body.substr(pos + 1).c_str();
            set(key, value, callback, onError);
            return true;
        }
    }
    std::cout << "unknown command \"" << requestData.data() << "\"" << std::endl;
    return false;
}

void Connection::Handler::send(const std::string &responseData, std::function<void()> callback, std::function<void(const boost::system::error_code &eCode)> onError)
{
    // add header size
    pkgsize_t sendDataSize = std::max<pkgsize_t>(responseData.size(), MIN_LENGTH) + PACKET_HEADER_SIZE;
    // align to PACKET_ALIGNMENT
    if (sendDataSize % PACKET_ALIGNMENT != 0) {
        sendDataSize += PACKET_ALIGNMENT - sendDataSize % PACKET_ALIGNMENT;
    }
    // prepare buffer
    std::vector<char> sendDataRaw(sendDataSize);
    pkgsize_t sendDataSizeNet = htonl(sendDataSize);
    memcpy(sendDataRaw.data(), &sendDataSizeNet, PACKET_HEADER_SIZE);
    memcpy(sendDataRaw.data() + PACKET_HEADER_SIZE, responseData.data(), responseData.size());
    auto sendBuffer = boost::asio::buffer(const_cast<const char*>(sendDataRaw.data()), sendDataRaw.size());
    // send
    boost::system::error_code eCode;
    this->sock.async_send(sendBuffer, 0, [this, self = this->shared_from_this(), callback, onError](const boost::system::error_code& eCode, std::size_t /*length*/) {
        if (!eCode || eCode == boost::asio::error::eof) {
            callback();
        }
        else {
            onError(eCode);
        }
    });
}

void Connection::Handler::get(const std::string &key, std::function<void()> callback, std::function<void(const boost::system::error_code &eCode)> onError)
{
    const std::string responseData = Database::Get(key);
    send(responseData, callback, onError);
}

void Connection::Handler::set(const std::string &key, const std::string &value, std::function<void()> callback, std::function<void(const boost::system::error_code &eCode)> onError)
{
    const std::string responseData = Database::Set(key, value);
    send(responseData, callback, onError);
}

boost::asio::ip::tcp::socket& Connection::Handler::getSocket()
{
    return this->sock;
}
void Connection::Handler::process()
{
    // I think this is to keep the shared count above zero so that the connection object is not destroyed)
    auto self = this->shared_from_this();

    // the actual read operation
    boost::system::error_code eCode;
    pkgsize_t responseSize = 0;
    boost::asio::mutable_buffer responseSizeBuffer(&responseSize, PACKET_HEADER_SIZE);
    std::size_t bytesReceived = sock.receive(responseSizeBuffer, 0, eCode);
    if (bytesReceived && (!eCode || (eCode == boost::asio::error::eof))) {
        responseSize = ntohl(responseSize);
        std::vector<char> requestData(responseSize - PACKET_HEADER_SIZE);
        boost::asio::mutable_buffer requestBuffer(requestData.data(), requestData.size());
        bytesReceived = sock.receive(requestBuffer, 0, eCode);
        if (bytesReceived && (!eCode || (eCode == boost::asio::error::eof))) {
            if (!requestData[0]) // close connection if request is empty
                this->sock.close();
            if (!this->sock.is_open()) // ignore if connection is closed
                return;

            // response
            this->processRequest(requestData, [this, self, &requestData]()
                {
                    requestData.clear();
                    requestData.shrink_to_fit();
                    this->process();
                },
                [this, self](const boost::system::error_code &eCode)
                {
                    this->sock.close();
                });
        }
        else {
            std::cerr << "error (receive): " << eCode.message() << std::endl;
        }
    }
    else {
        if (eCode != boost::asio::error::eof) {
            std::cerr << "error (receive): " << eCode.message() << std::endl;
        }
    }
}