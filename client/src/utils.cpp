#include "utils.hpp"
#include "resources.hpp"

const std::string Utils::generateRandomValue()
{
    const std::size_t valueLength = distValueLength(rng);
    std::string value = "";
    value.reserve(valueLength);
    for (std::size_t i = 0; i < valueLength; i++) {
        value += charsCollection[distChar(rng)];
    }
    return value;
}
