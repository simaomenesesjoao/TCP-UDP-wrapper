#include "../../src/server.hpp"
#include <iostream>

int main(int argc, char** argv){

    std::string portUDP = "8448";
    std::string portTCP = "8447";

    TCPServer TCP(portTCP, 1);
    int fd = TCP.accept_connection();
    TCPInstance connection(fd);

    UDPServer UDP(portUDP);

    // Inform the client about the UDP port available
    std::string msg = "OK,PORT="+portUDP;
    connection.send_data(msg.c_str(), msg.size()+1);
    
    // Receive acknowledgement from client. This step may be important for NAT traversal
    // Server already has an open port for UDP, so hope punching is not required
    // This step also sets the client's address in the UDP server
    char buffer[100];
    UDP.receive_data(buffer, 100);
    std::cout << "server received: " << std::string(buffer) << "\n";
    UDP.print_current_address();

    // Acknowledge UDP packet received
    msg = "UDP packet received";
    connection.send_data(msg.c_str(), msg.size()+1);


    connection.receive_data(buffer, 100);
    std::cout << "server received: " << std::string(buffer) << "\n";
    return 0;
}