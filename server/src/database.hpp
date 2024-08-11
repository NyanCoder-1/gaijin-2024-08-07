#pragma once

#include <string>

namespace Database {
    void Load();
    void Save();
    std::string Get(const std::string& key);
    std::string Set(const std::string& key, const std::string& value);
}
