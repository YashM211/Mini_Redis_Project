#include "KeyValueStore.h"

KeyValueStore::KeyValueStore(size_t capacity)
    : capacity(capacity) {}

void KeyValueStore::set(
    const std::string& key,
    const std::string& value
) {

    auto it = index.find(key);

    // Key already exists
    if (it != index.end()) {

        it->second->second = value;

        cache.splice(
            cache.begin(),
            cache,
            it->second
        );

        return;
    }

    // Handle zero capacity
    if (capacity == 0) {
        return;
    }

    // Evict if cache is full
    if (cache.size() >= capacity) {
        evict();
    }

    cache.push_front({key, value});

    index[key] = cache.begin();
}

std::optional<std::string> KeyValueStore::get(
    const std::string& key
) {

    auto it = index.find(key);

    if (it == index.end()) {
        return std::nullopt;
    }

    // Move accessed element to front
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

    cache.pop_back();
}

bool KeyValueStore::remove(const std::string& key) {

    auto it = index.find(key);

    if (it == index.end()) {
        return false;
    }

    cache.erase(it->second);

    index.erase(it);

    return true;
}

bool KeyValueStore::exists(const std::string& key) const {

    return index.find(key) != index.end();
}