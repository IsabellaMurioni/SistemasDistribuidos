#include "tcp_connection.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <iostream>

namespace net {

TcpConnection::TcpConnection() : socket_fd(-1), connected(false) {}

TcpConnection::TcpConnection(int fd) : socket_fd(fd), connected(true) {}

TcpConnection::~TcpConnection() {
    disconnect();
}

bool TcpConnection::connect(const std::string& host, int port) {
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        std::cerr << "Failed to create socket" << std::endl;
        return false;
    }

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    
    if (inet_pton(AF_INET, host.c_str(), &server_addr.sin_addr) <= 0) {
        std::cerr << "Invalid address: " << host << std::endl;
        close(socket_fd);
        socket_fd = -1;
        return false;
    }

    if (::connect(socket_fd, (sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Connection failed to " << host << ":" << port << std::endl;
        close(socket_fd);
        socket_fd = -1;
        return false;
    }

    connected = true;
    return true;
}

void TcpConnection::disconnect() {
    if (socket_fd >= 0) {
        close(socket_fd);
        socket_fd = -1;
    }
    connected = false;
}

bool TcpConnection::sendMessage(const std::string& message) {
    if (!connected) return false;

    // Send message length first (4 bytes)
    uint32_t length = htonl(message.length());
    if (!sendAll(reinterpret_cast<const char*>(&length), sizeof(length))) {
        return false;
    }

    // Send message data
    return sendAll(message.c_str(), message.length());
}

std::string TcpConnection::receiveMessage() {
    if (!connected) return "";

    // Receive message length first (4 bytes)
    auto length_data = receiveAll(sizeof(uint32_t));
    if (length_data.empty()) {
        return "";
    }

    uint32_t length = ntohl(*reinterpret_cast<uint32_t*>(length_data.data()));
    
    // Sanity check on message length
    if (length > 1024 * 1024) { // 1MB limit
        std::cerr << "Message too large: " << length << " bytes" << std::endl;
        return "";
    }

    // Receive message data
    auto message_data = receiveAll(length);
    if (message_data.empty()) {
        return "";
    }

    return std::string(message_data.data(), length);
}

bool TcpConnection::sendAll(const char* data, size_t length) {
    size_t sent = 0;
    while (sent < length) {
        ssize_t result = send(socket_fd, data + sent, length - sent, 0);
        if (result <= 0) {
            std::cerr << "Send failed" << std::endl;
            connected = false;
            return false;
        }
        sent += result;
    }
    return true;
}

std::vector<char> TcpConnection::receiveAll(size_t length) {
    std::vector<char> buffer(length);
    size_t received = 0;
    
    while (received < length) {
        ssize_t result = recv(socket_fd, buffer.data() + received, length - received, 0);
        if (result <= 0) {
            if (result == 0) {
                std::cerr << "Connection closed by peer" << std::endl;
            } else {
                std::cerr << "Receive failed" << std::endl;
            }
            connected = false;
            return {};
        }
        received += result;
    }
    
    return buffer;
}

// TcpServer implementation

TcpServer::TcpServer(int port) : server_fd(-1), port(port), running(false) {}

TcpServer::~TcpServer() {
    stop();
}

bool TcpServer::start() {
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        std::cerr << "Failed to create server socket" << std::endl;
        return false;
    }

    // Allow socket reuse
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        std::cerr << "setsockopt failed" << std::endl;
        close(server_fd);
        server_fd = -1;
        return false;
    }

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(server_fd, (sockaddr*)&address, sizeof(address)) < 0) {
        std::cerr << "Bind failed on port " << port << std::endl;
        close(server_fd);
        server_fd = -1;
        return false;
    }

    if (listen(server_fd, 10) < 0) {
        std::cerr << "Listen failed" << std::endl;
        close(server_fd);
        server_fd = -1;
        return false;
    }

    running = true;
    std::cout << "Server started on port " << port << std::endl;
    return true;
}

void TcpServer::stop() {
    running = false;
    if (server_fd >= 0) {
        close(server_fd);
        server_fd = -1;
    }
}

std::unique_ptr<TcpConnection> TcpServer::acceptConnection() {
    if (!running) return nullptr;

    sockaddr_in client_addr{};
    socklen_t addr_len = sizeof(client_addr);
    
    int client_fd = accept(server_fd, (sockaddr*)&client_addr, &addr_len);
    if (client_fd < 0) {
        if (running) { // Only log error if we're still supposed to be running
            std::cerr << "Accept failed" << std::endl;
        }
        return nullptr;
    }

    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
    std::cout << "New connection from " << client_ip << ":" << ntohs(client_addr.sin_port) << std::endl;

    return std::make_unique<TcpConnection>(client_fd);
}

} // namespace net