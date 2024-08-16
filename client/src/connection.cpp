#include "connection.hpp"
#include "resources.hpp"

Connection::Connection(Connection::Private) :
    sock(this->ioService)
{}

const boost::system::error_code Connection::connect(const std::string &host, boost::asio::ip::port_type port)
{
    if (sock.is_open()) {
        sock.close();
    }
    boost::system::error_code eCode;
    sock.connect(boost::asio::ip::tcp::endpoint(boost::asio::ip::address_v4::from_string(host), port), eCode);
    return eCode;
}

const bool Connection::isConnected() const
{
    return sock.is_open();
}

const std::tuple<const std::vector<char>, const boost::system::error_code> Connection::receive()
{
    boost::system::error_code eCode;
    pkgsize_t responseSize = 0;
    boost::asio::mutable_buffer responseSizeBuffer(&responseSize, PACKET_HEADER_SIZE);
    sock.receive(responseSizeBuffer, 0, eCode);
    if (!Connection::IsGood(eCode)) {
        return {{}, eCode};
    }
    responseSize = ntohl(responseSize);
    std::vector<char> responseData(responseSize - PACKET_HEADER_SIZE);
    boost::asio::mutable_buffer responseBuffer(responseData.data(), responseData.size());
    sock.receive(responseBuffer, 0, eCode);
    return {std::move(responseData), eCode};
}

const boost::system::error_code Connection::send(const std::vector<char> &data)
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
const boost::system::error_code Connection::send(const std::string &data)
{
    std::vector<char> sendData(data.begin(), data.end());
    return Connection::send(sendData);
}
