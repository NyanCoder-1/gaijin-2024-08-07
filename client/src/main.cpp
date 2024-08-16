#include "connection.hpp"
#include "resources.hpp"
#include "utils.hpp"
#include <chrono>
#include <iostream>
#include <thread>

//
// the client connection architecture was inspired from
// https://www.codeproject.com/Articles/1264257/Socket-Programming-in-Cplusplus-using-boost-asio-T
//

int main()
{
    auto connection = Connection::Create();
    boost::system::error_code eCode = connection->connect(HOST, PORT);
    std::string request = "";
    for (auto i = 0; i < REQUESTS_AMOUNT; i++) {
        auto printError = [connection, &i](
            const boost::system::error_code &eCode, const std::string &title,
            const bool excludeEof = false, const bool customError = false,
            const std::string customMessage = "") -> bool
        {
            if (!Connection::IsGood(eCode, excludeEof)) {
                std::cout << title << ": " << eCode.message() << std::endl;
                i--;
                return true;
            }
            if (customError) {
                std::cout << title << ": " << customMessage << std::endl;
                i--;
                return true;
            }
            return false;
        };

        // reconnect if needed
        bool firstReconnect = true;
        while (!connection->isConnected() || !Connection::IsGood(eCode)) {
            if (!firstReconnect) {
                std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            }
            std::cout << "remote is unreachable or request failed, reconnecting..." << std::endl;
            eCode = connection->connect(HOST, PORT);
            firstReconnect = false;
        }

        // prepare request
        std::string request = "";
        if (distProbability(rng)) { // GET
            request = REQUEST_GET + keys[distKey(rng)];
        }
        else { // SET
            auto value = Utils::generateRandomValue();
            request = REQUEST_SET + keys[distKey(rng)] + "=" + value;
        }
        eCode = connection->send(request);
        if (printError(eCode, "error sending request")) {
            continue;
        }

        // receive response
        std::vector<char> response;
        std::tie(response, eCode) = connection->receive();
        if (printError(eCode, "error receiving response", true, !response.size(), "response is empty")) {
            continue;
        }
        // print response
        auto responseStr = std::string(response.begin(), response.end());
        std::cout << responseStr << std::endl;
    }

    return 0;
}