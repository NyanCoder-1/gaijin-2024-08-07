#include "connection.hpp"
#include "database.hpp"
#include "resources.hpp"
#include "utils.hpp"
#include <cstdint>
#include <iostream>
#include <thread>

Connection::Handler::Handler(boost::asio::io_service& ioService, Connection::Handler::Private) :
    sock(ioService)
{}

const boost::system::error_code Connection::Handler::processRequest(const std::vector<char> &requestData)
{
    if ((requestData.size() > COMMAND_GET_LENGTH) &&
        !strncmp(requestData.data(), COMMAND_GET_START, COMMAND_GET_LENGTH)) {
        const auto request = std::string(requestData.data() + COMMAND_GET_LENGTH, requestData.size() - COMMAND_GET_LENGTH);
        return get(request.c_str());
    }
    else if ((requestData.size() > COMMAND_SET_LENGTH) &&
        !strncmp(requestData.data(), COMMAND_SET_START, COMMAND_SET_LENGTH)) {
        const auto body = std::string(requestData.data() + COMMAND_SET_LENGTH, requestData.size() - COMMAND_SET_LENGTH);
        const auto pos = body.find('=');
        if (pos != std::string::npos) {
            const std::string key = body.substr(0, pos).c_str();
            const std::string value = body.substr(pos + 1).c_str();
            return set(key, value);
        }
        else {
            return boost::system::error_code(static_cast<int>(ConnectionError::InvalidSet), ConnectionErrorCategory());
        }
    }
    return boost::system::error_code(static_cast<int>(ConnectionError::UnknownCommand), ConnectionErrorCategory());
}

const std::tuple<const std::vector<char>, boost::system::error_code> Connection::Handler::receive()
{
    boost::system::error_code eCode;
    pkgsize_t responseSize = 0;
    boost::asio::mutable_buffer responseSizeBuffer(&responseSize, PACKET_HEADER_SIZE);
    sock.receive(responseSizeBuffer, 0, eCode);
    if (eCode) {
        return {{}, eCode};
    }
    responseSize = ntohl(responseSize);
    std::vector<char> responseData(responseSize - PACKET_HEADER_SIZE);
    boost::asio::mutable_buffer responseBuffer(responseData.data(), responseData.size());
    sock.receive(responseBuffer, 0, eCode);
    return {std::move(responseData), eCode};
}

const boost::system::error_code Connection::Handler::send(const std::string &data)
{
    return Connection::Handler::send(std::vector<char>(data.begin(), data.end()));
}
const boost::system::error_code Connection::Handler::send(const std::vector<char> &data)
{
    pkgsize_t requestSize = std::max<pkgsize_t>(data.size() + PACKET_HEADER_SIZE, MIN_LENGTH);
    // align to PACKET_ALIGNMENT
    if (requestSize % PACKET_ALIGNMENT != 0) {
        requestSize += PACKET_ALIGNMENT - (requestSize % PACKET_ALIGNMENT);
    }
    std::vector<char> requestData(requestSize);

    // add header
    pkgsize_t requestSizeNet = htonl(requestSize);
    memcpy(requestData.data(), &requestSizeNet, PACKET_HEADER_SIZE);
    // add body
    memcpy(requestData.data() + PACKET_HEADER_SIZE, data.data(), data.size());
    boost::system::error_code eCode;
    sock.send(boost::asio::buffer(requestData), 0, eCode);
    if (eCode) {
        return eCode;
    }

    return boost::system::error_code();
}

const boost::system::error_code Connection::Handler::get(const std::string &key)
{
    return Connection::Handler::send(Database::Get(key));
}
const boost::system::error_code Connection::Handler::set(const std::string &key, const std::string &value)
{
    return Connection::Handler::send(Database::Set(key, value));
}

boost::asio::ip::tcp::socket& Connection::Handler::getSocket()
{
    return this->sock;
}

void Connection::Handler::process()
{
    // I believe this is to keep the shared count above zero so that the connection object is not destroyed
    auto self = this->shared_from_this();

    std::thread acceptThread([self, this]() {
        while (this->sock.is_open()) {
            auto [requestData, eCode] = Connection::Handler::receive();
            if (eCode || requestData.empty()) {
                this->sock.close();
                return;
            }

            eCode = Connection::Handler::processRequest(requestData);
            if (eCode) {
                this->sock.close();
                return;
            }
        }
    });
    acceptThread.detach();
}