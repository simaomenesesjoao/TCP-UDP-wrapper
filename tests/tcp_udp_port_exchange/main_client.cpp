#include "../../src/client.hpp"


int main(int argc, char** argv){

    std::string portTCP = "8447";
    std::string IP = "194.164.120.17";
    TCPClient TCP(IP, portTCP);
    TCP.continuous_connect();

    char buffer[100];
    TCP.receive_data(buffer, 100);
    std::string response = std::string(buffer);
    std::cout << "Received: " << response << "\n";

    std::string portUDP = response.substr( response.find("=")+1, response.size());
    std::cout << "UDP port received: " << portUDP << "\n";

    UDPClient UDP(IP, portUDP);
    std::string msg = "UDP OK";
    UDP.send_data(msg.c_str(), msg.size()+1);

    // Receive acknowledgement that UDP packet was received
    TCP.receive_data(buffer, 100);
    response = std::string(buffer);
    std::cout << "Received: " << response << "\n";

    // Tell the server that the client is ready
    msg = "Client ready";
    TCP.send_data(msg.c_str(), msg.size()+1);
    
    
    return 0;
}