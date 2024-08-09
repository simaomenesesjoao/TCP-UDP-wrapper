#include "../../src/server.hpp"
#include <iostream>

int main(int argc, char** argv){

    TCPServer server("8447",1);
    int fd = server.accept_connection();
    TCPInstance connection(fd);
    

    int data[5];
    connection.receive_data(data, 5*sizeof(int));
    
    std::cout << "Message:\n"; 
    for(int i=0; i<5; i++)
        std::cout << data[i] << " ";
    std::cout << "\n";

    return 0;
}