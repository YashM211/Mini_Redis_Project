#include <iostream>
#include <string>
#include <cerrno>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

constexpr int PORT = 6379;
constexpr size_t BUFFER_SIZE = 1024;
constexpr size_t MAX_COMMAND_SIZE = 4096;

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

// Processes multiple newline-delimited messages.
void handleClient(int client_fd) {
    char buffer[BUFFER_SIZE];
    std::string pending;

    while (true) {
        ssize_t bytes_received = recv(
            client_fd,
            buffer,
            sizeof(buffer),
            0
        );

        if (bytes_received < 0) {
            if (errno == EINTR) continue;

            perror("Receive failed");
            break;
        }

        if (bytes_received == 0) {
            std::cout << "Client disconnected." << std::endl;
            break;
        }

        pending.append(buffer, static_cast<size_t>(bytes_received));

        // Process every complete message.
        size_t delimiter;

        while ((delimiter = pending.find('\n')) != std::string::npos) {

            std::string message = pending.substr(0, delimiter);

            pending.erase(0, delimiter + 1);

            // Support CRLF as well as LF.
            if (!message.empty() && message.back() == '\r') {
                message.pop_back();
            }

            std::cout << "Received: " << message << std::endl;

            std::string response = "Server received: " + message + "\n";

            if (!sendAll(client_fd, response)) {
                std::cerr << "Send failed." << std::endl;
                return;
            }
        }

        // Prevent unbounded memory consumption.
        if (pending.size() > MAX_COMMAND_SIZE) {
            sendAll(client_fd, "ERROR: Command too large\n");
            break;
        }
    }
}

int main() {

    // Create TCP socket.
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd == -1) {
        perror("Socket creation failed");
        return 1;
    }

    // Enable address reuse.
    int opt = 1;

    if (setsockopt(
        server_fd,
        SOL_SOCKET,
        SO_REUSEADDR,
        &opt,
        sizeof(opt)
    ) < 0) {
        perror("setsockopt failed");
        close(server_fd);
        return 1;
    }

    // Configure server address.
    sockaddr_in address{};

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Bind.
    if (bind(
        server_fd,
        reinterpret_cast<sockaddr*>(&address),
        sizeof(address)
    ) < 0) {
        perror("Bind failed");
        close(server_fd);
        return 1;
    }

    // Listen.
    if (listen(server_fd, 10) < 0) {
        perror("Listen failed");
        close(server_fd);
        return 1;
    }

    std::cout << "MiniRedis server listening on port "
              << PORT << std::endl;

    // Accept clients.
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

        std::cout << "Client connected!" << std::endl;

        handleClient(client_fd);

        close(client_fd);
    }

    close(server_fd);
    return 0;
}