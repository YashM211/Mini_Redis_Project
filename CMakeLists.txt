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
)

# TCP Client
add_executable(miniredis_client
    src/Client.cpp
)

target_include_directories(miniredis PRIVATE include)