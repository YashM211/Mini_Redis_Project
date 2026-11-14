#ifndef KEY_VALUE_STORE_H
#define KEY_VALUE_STORE_H

#include <string>
#include <unordered_map>
#include <optional>
#include <list>

class KeyValueStore {

private:

    using Entry = std::pair<std::string, std::string>;

    size_t capacity;

    std::list<Entry> cache;

    std::unordered_map<
        std::string,
        std::list<Entry>::iterator
    > index;

    void evict();

public:

    explicit KeyValueStore(size_t capacity = 1000);

    void set(const std::string& key, const std::string& value);

    std::optional<std::string> get(const std::string& key);

    bool remove(const std::string& key);

    bool exists(const std::string& key) const;

};

#endif