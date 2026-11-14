#include <iostream>
#include "KeyValueStore.h"

int main() {

    KeyValueStore db(3);

    db.set("A", "10");
    db.set("B", "20");
    db.set("C", "30");

    std::cout << "A: " << *db.get("A") << '\n';

    db.set("D", "40");

    std::cout << "B exists: " << db.exists("B") << '\n';
    std::cout << "C exists: " << db.exists("C") << '\n';
    std::cout << "D exists: " << db.exists("D") << '\n';

    return 0;
}