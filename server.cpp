#include "common/tcp_connection.h"
#include "common/rpc_protocol.h"
#include <iostream>
using namespace std ;


int main() {
    
    // GLHF
    net::TcpServer server  ;
        server.start();
        std::unique_ptr<net::TcpConnection> connection = server.acceptConnection();
        cout << connection->receiveMessage() << endl;
        connection->sendMessage(rpc::void_response("1"));

    return 0;
}