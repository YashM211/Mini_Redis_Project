#include <iostream>
#include <string>
#include <sstream>
#include <cerrno>
#include <optional>
#include <chrono>

#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <thread>

#include "KeyValueStore.h"

constexpr int PORT = 6379;
constexpr size_t BUFFER_SIZE = 1024;
constexpr size_t MAX_COMMAND_SIZE = 4096;

// Shared database
KeyValueStore db(1000);


// Send complete response
bool sendAll(int socket_fd, const std::string& message) {

    size_t total_sent = 0;

    while (total_sent < message.size()) {

        ssize_t sent = send(
            socket_fd,
            message.data() + total_sent,
            message.size() - total_sent,
            MSG_NOSIGNAL
        );

        if (sent < 0) {
            if (errno == EINTR) continue;
            return false;
        }

        if (sent == 0) return false;

        total_sent += static_cast<size_t>(sent);
    }

    return true;
}


// Process Redis commands
std::string executeCommand(const std::string& command) {

    std::istringstream iss(command);

    std::string operation;
    iss >> operation;

    if (operation == "SET") {

        std::string key, value, option;

        if (!(iss >> key >> value)) {
            return "ERROR: Invalid SET command\n";
        }

        std::optional<std::chrono::seconds> ttl = std::nullopt;

        if (iss >> option) {

            std::string seconds;

            if (option != "EX" || !(iss >> seconds)) {
                return "ERROR: Expected EX seconds\n";
            }

            try {
                size_t pos = 0;
                long long duration = std::stoll(seconds, &pos);

                if (pos != seconds.size() || duration <= 0) {
                    return "ERROR: Invalid expiration time\n";
                }

                std::string extra;

                if (iss >> extra) {
                    return "ERROR: Too many arguments\n";
                }

                ttl = std::chrono::seconds(duration);

            } catch (...) {
                return "ERROR: Invalid expiration time\n";
            }
        }

        db.set(key, value, ttl);

        return "OK\n";
    }

    if (operation == "GET") {

        std::string key, extra;

        if (!(iss >> key) || (iss >> extra)) {
            return "ERROR: Invalid GET command\n";
        }

        auto value = db.get(key);

        if (!value.has_value()) {
            return "(nil)\n";
        }

        return value.value() + "\n";
    }

    if (operation == "DELETE" || operation == "DEL") {

        std::string key, extra;

        if (!(iss >> key) || (iss >> extra)) {
            return "ERROR: Invalid DELETE command\n";
        }

        return db.remove(key) ? "OK\n" : "0\n";
    }

    if (operation == "EXISTS") {

        std::string key, extra;

        if (!(iss >> key) || (iss >> extra)) {
            return "ERROR: Invalid EXISTS command\n";
        }

        return db.exists(key) ? "1\n" : "0\n";
    }

    if (operation == "PING") {
        return "PONG\n";
    }

    return "ERROR: Unknown command\n";
}


// Handle client requests
void handleClient(int client_fd) {

    char buffer[BUFFER_SIZE];

    std::string pending;

    while (true) {

        ssize_t received = recv(
            client_fd,
            buffer,
            sizeof(buffer),
            0
        );

        if (received < 0) {

            if (errno == EINTR) continue;

            perror("Receive failed");
            break;
        }

        if (received == 0) {
            std::cout << "Client disconnected\n";
            break;
        }

        pending.append(buffer, static_cast<size_t>(received));

        size_t delimiter;

        while ((delimiter = pending.find('\n')) != std::string::npos) {

            std::string command = pending.substr(0, delimiter);

            pending.erase(0, delimiter + 1);

            if (!command.empty() && command.back() == '\r') {
                command.pop_back();
            }

            std::cout << "Command: " << command << '\n';

            std::string response = executeCommand(command);

            if (!sendAll(client_fd, response)) {
                return;
            }
        }

        if (pending.size() > MAX_COMMAND_SIZE) {

            sendAll(client_fd, "ERROR: Command too large\n");
            break;
        }
    }
}


int main() {

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd == -1) {
        perror("Socket creation failed");
        return 1;
    }

    int opt = 1;

    setsockopt(
        server_fd,
        SOL_SOCKET,
        SO_REUSEADDR,
        &opt,
        sizeof(opt)
    );

    sockaddr_in address{};

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(
        server_fd,
        reinterpret_cast<sockaddr*>(&address),
        sizeof(address)
    ) < 0) {

        perror("Bind failed");
        close(server_fd);
        return 1;
    }

    if (listen(server_fd, 10) < 0) {

        perror("Listen failed");
        close(server_fd);
        return 1;
    }

    std::cout << "MiniRedis server listening on port "
              << PORT << '\n';

    while (true) {

        sockaddr_in client_address{};
        socklen_t client_len = sizeof(client_address);

        int client_fd = accept(
            server_fd,
            reinterpret_cast<sockaddr*>(&client_address),
            &client_len
        );

        if (client_fd < 0) {

            if (errno == EINTR) continue;

            perror("Accept failed");
            continue;
        }

        std::cout << "Client connected\n";

        std::thread clientThread([client_fd]() {

            handleClient(client_fd);

            close(client_fd);

        });

        clientThread.detach();
    }

    close(server_fd);

    return 0;
}