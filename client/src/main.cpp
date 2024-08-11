#include <arpa/inet.h>
#include <array>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <random>
#include <string>
#include <thread>
#include <boost/asio.hpp>

// server address
constexpr auto HOST = "127.0.0.1";
constexpr auto PORT = 8000;

// buffer config
constexpr auto MAX_LENGTH = 1024;
constexpr auto MIN_LENGTH = 16;
constexpr auto PACKET_ALIGNMENT = 4;
typedef std::uint32_t pkgsize_t;
constexpr auto PACKET_HEADER_SIZE = sizeof(pkgsize_t);

// randomizer
std::random_device dev;
std::mt19937 rng(dev());

// requests helper data
constexpr auto REQUEST_GET = "$get ";
constexpr auto REQUEST_SET = "$set ";
std::uniform_int_distribution<std::mt19937::result_type> distProbability(0, 99);
constexpr auto KEYS_AMOUNT = 10;
const std::array<std::string, KEYS_AMOUNT> keys {
    "key1", "key2", "key3", "key4", "key5", "key6", "key7", "key8", "key9", "key10"
};
std::uniform_int_distribution<std::mt19937::result_type> distKey(0, keys.size() - 1);
const std::array<char, 62> charsCollection = {
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z'
};
std::uniform_int_distribution<std::mt19937::result_type> distChar(0, charsCollection.size() - 1);
std::uniform_int_distribution<std::mt19937::result_type> distValueLength(5, 32);

constexpr auto REQUESTS_AMOUNT = 10000;

//
// the client connection architecture was inspired from
// https://www.codeproject.com/Articles/1264257/Socket-Programming-in-Cplusplus-using-boost-asio-T
//

int main()
{
    boost::asio::io_service ioService;
    boost::asio::ip::tcp::socket sock(ioService);
    boost::system::error_code eCode;
    sock.connect(boost::asio::ip::tcp::endpoint(boost::asio::ip::address_v4::from_string(HOST), PORT), eCode);
    std::string request = "";
    for (auto i = 0; i < REQUESTS_AMOUNT; i++) {
        std::vector<char> requestData(MAX_LENGTH);
        while (!sock.is_open() || eCode) {
            if (sock.is_open()) {
                sock.close();
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            std::cout << "remote is unreachable, trying again..." << std::endl;
            sock.connect(boost::asio::ip::tcp::endpoint(boost::asio::ip::address_v4::from_string(HOST), PORT), eCode);
        }

        pkgsize_t requestSize = MIN_LENGTH;
        // prepare request body
        if (distProbability(rng)) {
            request = REQUEST_GET + keys[distKey(rng)];
            requestSize = std::max<std::size_t>(request.size(), MIN_LENGTH) + PACKET_HEADER_SIZE;
        }
        else {
            const std::size_t valueLength = distValueLength(rng);
            std::string value = "";
            value.reserve(valueLength);
            for (std::size_t i = 0; i < valueLength; i++) {
                value += charsCollection[distChar(rng)];
            }
            request = REQUEST_SET + keys[distKey(rng)] + "=" + value;
            requestSize = std::max<std::size_t>(request.size(), MIN_LENGTH) + PACKET_HEADER_SIZE;
        }
        if (requestSize % PACKET_ALIGNMENT != 0) {
            requestSize += PACKET_ALIGNMENT - (requestSize % PACKET_ALIGNMENT);
        }
        pkgsize_t requestSizeNet = htonl(requestSize);
        memcpy(requestData.data(), &requestSizeNet, PACKET_HEADER_SIZE);
        memcpy(requestData.data() + PACKET_HEADER_SIZE, request.c_str(), request.size());

        // the request
        boost::asio::error::clear(eCode);
        sock.send(boost::asio::buffer(requestData.data(), requestSize), boost::asio::socket_base::message_end_of_record, eCode);
        if (eCode) {
            std::cout << "error (send failed): " << eCode << std::endl;
            i--;
            continue;
        }

        // the response
        pkgsize_t responseSize = 0;
        boost::asio::mutable_buffer responseSizeBuffer(&responseSize, PACKET_HEADER_SIZE);
        boost::asio::error::clear(eCode);
        std::size_t bytesReceived = sock.receive(responseSizeBuffer, 0, eCode);
        if (!bytesReceived || (eCode && (eCode != boost::asio::error::eof))) {
            std::cout << "error (receive failed): " << eCode << std::endl;
            i--;
            continue;
        }
        responseSize = ntohl(responseSize);
        std::vector<char> responseData(responseSize - PACKET_HEADER_SIZE);
        boost::asio::mutable_buffer responseBuffer(responseData.data(), responseData.size());
        boost::asio::error::clear(eCode);
        bytesReceived = sock.receive(responseBuffer, 0, eCode);
        if (!bytesReceived || (eCode && (eCode != boost::asio::error::eof))) {
            std::cout << "error (receive failed): " << eCode << std::endl;
            i--;
            continue;
        }
        else {
            std::string responseStr = std::string(responseData.begin(), responseData.end());
            std::cout << responseStr << std::endl << std::endl;
        }
    }

    return 0;
}