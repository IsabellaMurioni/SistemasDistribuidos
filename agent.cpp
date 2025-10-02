#include "common/tcp_connection.h"
int main() {
    // GLHF 
    net::TcpConnection conecction  ;
    conecction.connect("192.168.56.1" ,8080);
    return 0;
}