#include "../../src/server.hpp"
#include <iostream>

int main(int argc, char** argv){

    std::string portUDP = "8448";
    std::string portTCP = "8447";

    TCPServer TCP(portTCP, 1);
    int fd = TCP.accept_connection();
    TCPInstance connection(fd);

    UDPServer UDP(portUDP);

    std::string msg = "OK,PORT="+portUDP;
    connection.send_data(msg.c_str(), msg.size()+1);
    

    char buffer[100];
    UDP.receive_data(buffer, 100);
    std::cout << "server received: " << std::string(buffer) << "\n";



    // UDPServer server("8448",1);

    // int data[5];
    // server.print_current_address();
    // server.receive_data(data, 5*sizeof(int));
    // std::cout << "Message received:"; 
    // for(int i=0; i<5; i++) std::cout << data[i];
    // std::cout << "\n";


    // std::cout << "Sending doubled data back\n";
    // for(int i=0; i<5; i++)data[i] *= 2;
    // server.print_current_address();
    // server.send_data(data, 5*sizeof(int));
    // server.print_current_address();



    // server.receive_data(data, 5*sizeof(int));
    // server.print_current_address();
    // std::cout << "Message received:"; 
    // for(int i=0; i<5; i++)std::cout << data[i] << " ";
    // std::cout << "\n";

    // return 0;
}