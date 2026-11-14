#ifndef KEY_VALUE_STORE_H
#define KEY_VALUE_STORE_H

#include <string>
#include <unordered_map>
#include <optional>
#include <list>
#include <chrono>

class KeyValueStore {

private:

    using Entry = std::pair<std::string, std::string>;

    using Clock = std::chrono::steady_clock;
    using TimePoint = Clock::time_point;

    size_t capacity;

    std::list<Entry> cache;

    std::unordered_map<
        std::string,
        std::list<Entry>::iterator
    > index;

    std::unordered_map<std::string, TimePoint> expiry;

    void evict();

    bool expireIfNeeded(const std::string& key);

public:

    explicit KeyValueStore(size_t capacity = 1000);

    void set(
        const std::string& key,
        const std::string& value,
        std::optional<std::chrono::seconds> ttl = std::nullopt
    );

    std::optional<std::string> get(const std::string& key);

    bool remove(const std::string& key);

    bool exists(const std::string& key);

};

#endif