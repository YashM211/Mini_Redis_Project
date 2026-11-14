#include "KeyValueStore.h"

void KeyValueStore::set(const std::string& key,
                        const std::string& value) {
    store[key] = value;
}

std::optional<std::string> KeyValueStore::get(
    const std::string& key) {

    auto it = store.find(key);

    if (it == store.end()) {
        return std::nullopt;
    }

    return it->second;
}

bool KeyValueStore::remove(const std::string& key) {
    return store.erase(key) > 0;
}

bool KeyValueStore::exists(const std::string& key) {
    return store.find(key) != store.end();
}