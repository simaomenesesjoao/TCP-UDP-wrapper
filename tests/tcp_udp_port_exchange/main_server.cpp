#include "../../src/server.hpp"
#include <iostream>

int main(int argc, char** argv){

    std::string portUDP = "8448";
    std::string portTCP = "8447";

    TCPServer TCP(portTCP, 1);
    int fd = TCP.accept_connection();

    MultiServer server(fd, portUDP);
    server.handshake();


    
   
    return 0;
}