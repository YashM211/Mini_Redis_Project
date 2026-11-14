#include <iostream>
#include "KeyValueStore.h"

int main() {
    KeyValueStore db;

    db.set("name", "Yash");
    db.set("language", "C++");

    std::cout << "Name: " << db.get("name") << '\n';
    std::cout << "Language: " << db.get("language") << '\n';

    std::cout << "Exists: " << db.exists("name") << '\n';

    db.remove("name");

    std::cout << "Exists after deletion: "
              << db.exists("name") << '\n';

    return 0;
}