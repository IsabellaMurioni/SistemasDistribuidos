// common/tcp_connection.h
// Declare the TcpConnection/TcpServer classes for the tcp_connection.cpp
#pragma once
#include <string>
#include <vector>
#include <memory>
#include <functional>

namespace net {

class TcpConnection {
private:
    int socket_fd;
    bool connected;

public:
    TcpConnection();
    explicit TcpConnection(int fd);
    ~TcpConnection();

    // Client operations
    bool connect(const std::string& host, int port);
    void disconnect();

    // Send/receive operations
    bool sendMessage(const std::string& message);
    std::string receiveMessage();
    
    // Status
    bool isConnected() const { return connected; }
    int getSocketFd() const { return socket_fd; }

private:
    bool sendAll(const char* data, size_t length);
    std::vector<char> receiveAll(size_t length);
};

class TcpServer {
private:
    int server_fd;
    int port;
    bool running;

public:
    explicit TcpServer(int port = 8080);
    ~TcpServer();

    bool start();
    void stop();
    
    // Accept new connections
    std::unique_ptr<TcpConnection> acceptConnection();
    
    // Status
    bool isRunning() const { return running; }
    int getPort() const { return port; }
};

} // namespace net