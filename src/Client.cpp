#include <iostream>
#include <string>
#include <cerrno>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

constexpr int PORT = 6379;

// Handles partial writes.
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

// Reads one newline-delimited response.
bool receiveLine(int socket_fd, std::string& response) {
    response.clear();

    while (true) {
        char ch;

        ssize_t received = recv(socket_fd, &ch, 1, 0);

        if (received < 0) {
            if (errno == EINTR) continue;
            return false;
        }

        if (received == 0) {
            return false;
        }

        if (ch == '\n') {
            return true;
        }

        response.push_back(ch);

        if (response.size() > 4096) {
            std::cerr << "Response too large." << std::endl;
            return false;
        }
    }
}

int main() {

    // Create socket.
    int client_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (client_fd == -1) {
        perror("Socket creation failed");
        return 1;
    }

    // Configure server address.
    sockaddr_in server_address{};

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);

    if (inet_pton(
        AF_INET,
        "127.0.0.1",
        &server_address.sin_addr
    ) != 1) {
        std::cerr << "Invalid server address." << std::endl;
        close(client_fd);
        return 1;
    }

    // Connect.
    if (connect(
        client_fd,
        reinterpret_cast<sockaddr*>(&server_address),
        sizeof(server_address)
    ) < 0) {
        perror("Connection failed");
        close(client_fd);
        return 1;
    }

    std::cout << "Connected to MiniRedis!" << std::endl;
    std::cout << "Type 'exit' to disconnect.\n" << std::endl;

    std::string command;

    while (true) {

        std::cout << "MiniRedis> ";

        if (!std::getline(std::cin, command)) {
            break;
        }

        if (command == "exit") {
            break;
        }

        command.push_back('\n');

        if (!sendAll(client_fd, command)) {
            std::cerr << "Failed to send command." << std::endl;
            break;
        }

        std::string response;

        if (!receiveLine(client_fd, response)) {
            std::cerr << "Server disconnected or response failed."
                      << std::endl;
            break;
        }

        std::cout << response << std::endl;
    }

    close(client_fd);

    std::cout << "Disconnected." << std::endl;

    return 0;
}