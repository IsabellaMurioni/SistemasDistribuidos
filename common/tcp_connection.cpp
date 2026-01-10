// common/tcp_connection.cpp
// Implements the TCP connection and server classes for network communication
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
    disconnect(); // Ensure socket is closed
}

bool TcpConnection::connect(const std::string& host, int port) { // Client-side connect
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) { // If socket < 0 it failed to be created
        std::cerr << "Failed to create socket" << std::endl;
        return false;
    }

    sockaddr_in server_addr{}; // Set up the server address structure
    server_addr.sin_family = AF_INET; // IPv4 obligatory
    server_addr.sin_port = htons(port);
    
    if (inet_pton(AF_INET, host.c_str(), &server_addr.sin_addr) <= 0) {
        std::cerr << "Invalid address: " << host << std::endl; // Convert IPv4 and IPv6 addresses from text to binary form
        close(socket_fd);
        socket_fd = -1;
        return false;
    }

    if (::connect(socket_fd, (sockaddr*)&server_addr, sizeof(server_addr)) < 0) { // Connect to server with socket_fd
        std::cerr << "Connection failed to " << host << ":" << port << std::endl;
        close(socket_fd);
        socket_fd = -1; // Mark as closed
        return false;
    }

    connected = true;
    return true;
}

void TcpConnection::disconnect() { // Close the connection
    if (socket_fd >= 0) { // If socket >= 0 it was successfully created and it has to be closed
        close(socket_fd);
        socket_fd = -1; // Mark as closed
    }
    connected = false;
}

bool TcpConnection::sendMessage(const std::string& message) {
    if (!connected) return false; // Very connection status is active

    // Send message length first (4 bytes)
    uint32_t length = htonl(message.length());
    if (!sendAll(reinterpret_cast<const char*>(&length), sizeof(length))) {
        return false;
    }

    // Send message data
    return sendAll(message.c_str(), message.length());
}

std::string TcpConnection::receiveMessage() {
    if (!connected) return ""; // Very connection status is active

    // Receive message length first (4 bytes)
    auto length_data = receiveAll(sizeof(uint32_t));
    if (length_data.empty()) {
        return ""; // Error or connection closed
    }

    uint32_t length = ntohl(*reinterpret_cast<uint32_t*>(length_data.data())); // Convert from network byte order to host byte order
    
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

    return std::string(message_data.data(), length); // Construct string from received data
}

bool TcpConnection::sendAll(const char* data, size_t length) { // Ensure all data is sent
    size_t sent = 0; // Track how many bytes have been sent
    while (sent < length) { // While not all data is sent
        ssize_t result = send(socket_fd, data + sent, length - sent, 0); // Send remaining data
        if (result <= 0) { // Error or connection closed
            std::cerr << "Send failed" << std::endl;
            connected = false;
            return false;
        }
        sent += result; 
    }
    return true; // All data sent successfully
}

std::vector<char> TcpConnection::receiveAll(size_t length) { // Ensure all data is received
    std::vector<char> buffer(length); // Buffer to hold received data
    size_t received = 0; // Track how many bytes have been received
    
    while (received < length) { // While not all data is received
        ssize_t result = recv(socket_fd, buffer.data() + received, length - received, 0); // Receive remaining data
        if (result <= 0) { // Error or connection closed
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
    
    return buffer; // All data received successfully
}

// TcpServer implementation

TcpServer::TcpServer(int port) : server_fd(-1), port(port), running(false) {} // Server_fd(-1) means socket not created


TcpServer::~TcpServer() {
    stop();
}

bool TcpServer::start() { 
    server_fd = socket(AF_INET, SOCK_STREAM, 0); // Create socket
    if (server_fd < 0) { // If socket < 0 it failed to be created
        std::cerr << "Failed to create server socket" << std::endl;
        return false;
    }

    // Allow socket reuse
    int opt = 1; // Enable address reuse
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) { // Set socket options
        std::cerr << "setsockopt failed" << std::endl;
        close(server_fd);
        server_fd = -1;
        return false;
    }

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(server_fd, (sockaddr*)&address, sizeof(address)) < 0) { // Associate socket with port
        std::cerr << "Bind failed on port " << port << std::endl;
        close(server_fd);
        server_fd = -1;
        return false;
    }

    if (listen(server_fd, 10) < 0) { // Start listening for connections
        std::cerr << "Listen failed" << std::endl;
        close(server_fd);
        server_fd = -1;
        return false;
    }

    running = true; // Mark server as running
    std::cout << "Server started on port " << port << std::endl;
    return true; // Server started successfully
}

void TcpServer::stop() {
    running = false; // Mark server as not running
    if (server_fd >= 0) { // If socket >= 0 it was successfully created and it has to be closed
        close(server_fd);
        server_fd = -1;
    }
}

std::unique_ptr<TcpConnection> TcpServer::acceptConnection() { // Accept a new client connection
    if (!running) return nullptr; // Server must be running to accept connections

    sockaddr_in client_addr{};
    socklen_t addr_len = sizeof(client_addr); 
    
    int client_fd = accept(server_fd, (sockaddr*)&client_addr, &addr_len); // Accept new connection
    if (client_fd < 0) { // Error accepting connection
        if (running) { // Only log error if we're still supposed to be running
            std::cerr << "Accept failed" << std::endl;
        }
        return nullptr; // Return null on failure
    }

    char client_ip[INET_ADDRSTRLEN]; // Convert client address to string
    inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
    std::cout << "New connection from " << client_ip << ":" << ntohs(client_addr.sin_port) << std::endl;

    return std::make_unique<TcpConnection>(client_fd); // Return new TcpConnection object
}

} // namespace net