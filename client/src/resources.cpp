#include "resources.hpp"

// randomizer
std::random_device dev;
std::mt19937 rng(dev());

// requests helper data
std::uniform_int_distribution<std::mt19937::result_type> distProbability(0, 99);
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
