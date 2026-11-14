#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define PORT 6379
#define BUFFER_SIZE 1024

int main() {

    // 1. Create client socket
    int client_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (client_fd == -1) {
        perror("Socket creation failed");
        return 1;
    }

    // 2. Configure server address
    sockaddr_in server_address{};

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);

    inet_pton(
        AF_INET,
        "127.0.0.1",
        &server_address.sin_addr
    );

    // 3. Connect to server
    if (connect(
        client_fd,
        (struct sockaddr*)&server_address,
        sizeof(server_address)
    ) < 0) {

        perror("Connection failed");
        close(client_fd);
        return 1;
    }

    std::cout << "Connected to MiniRedis Server!" << std::endl;

    // 4. Send message
    std::string message = "Hello MiniRedis!";

    send(
        client_fd,
        message.c_str(),
        message.size(),
        0
    );

    // 5. Receive response
    char buffer[BUFFER_SIZE];

    ssize_t bytes_received = recv(
        client_fd,
        buffer,
        BUFFER_SIZE,
        0
    );

    if (bytes_received > 0) {
        std::cout << "Response: "
                  << std::string(buffer, bytes_received)
                  << std::endl;
    }

    close(client_fd);

    return 0;
}