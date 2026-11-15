cmake_minimum_required(VERSION 3.16)

project(MiniRedis)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Core database
add_executable(miniredis
    src/main.cpp
    src/KeyValueStore.cpp
)

# TCP Server
add_executable(miniredis_server
    src/Server.cpp
    src/KeyValueStore.cpp
)

# TCP Client
add_executable(miniredis_client
    src/Client.cpp
)

# Include directories
target_include_directories(miniredis PRIVATE include)
target_include_directories(miniredis_server PRIVATE include)

find_package(Threads REQUIRED)

target_link_libraries(
    miniredis_server
    PRIVATE Threads::Threads
)