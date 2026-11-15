#include "KeyValueStore.h"

KeyValueStore::KeyValueStore(size_t capacity)
    : capacity(capacity) {}


bool KeyValueStore::expireIfNeeded(const std::string& key) {

    auto it = expiry.find(key);

    if (it == expiry.end()) {
        return false;
    }

    if (Clock::now() < it->second) {
        return false;
    }

    // Remove expired key from LRU cache
    auto cacheIt = index.find(key);

    if (cacheIt != index.end()) {
        cache.erase(cacheIt->second);
        index.erase(cacheIt);
    }

    expiry.erase(it);

    return true;
}


void KeyValueStore::set(
    const std::string& key,
    const std::string& value,
    std::optional<std::chrono::seconds> ttl
) {

    std::lock_guard<std::mutex> lock(mtx);

    // Remove the key if it has already expired
    expireIfNeeded(key);

    auto it = index.find(key);

    // Update existing key
    if (it != index.end()) {

        it->second->second = value;

        cache.splice(
            cache.begin(),
            cache,
            it->second
        );

    } else {

        if (capacity == 0) {
            return;
        }

        if (cache.size() >= capacity) {
            evict();
        }

        cache.push_front({key, value});

        index[key] = cache.begin();
    }

    // Update TTL
    if (ttl.has_value()) {

        expiry[key] = Clock::now() + ttl.value();

    } else {

        expiry.erase(key);
    }
}


std::optional<std::string> KeyValueStore::get(
    const std::string& key
) {

    std::lock_guard<std::mutex> lock(mtx);

    if (expireIfNeeded(key)) {
        return std::nullopt;
    }

    auto it = index.find(key);

    if (it == index.end()) {
        return std::nullopt;
    }

    cache.splice(
        cache.begin(),
        cache,
        it->second
    );

    return it->second->second;
}


void KeyValueStore::evict() {


    if (cache.empty()) {
        return;
    }

    auto last = cache.back();

    index.erase(last.first);

    expiry.erase(last.first);

    cache.pop_back();
}


bool KeyValueStore::remove(const std::string& key) {

    std::lock_guard<std::mutex> lock(mtx);

    auto it = index.find(key);

    if (it == index.end()) {
        expiry.erase(key);
        return false;
    }

    cache.erase(it->second);

    index.erase(it);

    expiry.erase(key);

    return true;
}


bool KeyValueStore::exists(const std::string& key) {

    std::lock_guard<std::mutex> lock(mtx);

    if (expireIfNeeded(key)) {
        return false;
    }

    return index.find(key) != index.end();
}