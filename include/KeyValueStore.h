#ifndef KEY_VALUE_STORE_H
#define KEY_VALUE_STORE_H

#include <string>
#include <unordered_map>

class KeyValueStore {
private:
    std::unordered_map<std::string, std::string> store;

public:
    void set(const std::string& key, const std::string& value);

    std::string get(const std::string& key);

    bool remove(const std::string& key);

    bool exists(const std::string& key);
};

#endif