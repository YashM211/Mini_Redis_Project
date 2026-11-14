#include <iostream>
#include <thread>
#include "KeyValueStore.h"

int main() {

    KeyValueStore db(3);

    // Normal key
    db.set("name", "Yash");

    // Key expires after 3 seconds
    db.set("session", "active", std::chrono::seconds(3));

    std::cout << "Name: " << *db.get("name") << '\n';

    std::cout << "Session exists: "
              << db.exists("session") << '\n';

    std::cout << "\nWaiting for 4 seconds...\n";

    std::this_thread::sleep_for(std::chrono::seconds(4));

    std::cout << "Session exists after expiration: "
              << db.exists("session") << '\n';

    std::cout << "Session value: "
              << (db.get("session").value_or("Key expired"))
              << '\n';

    return 0;
}