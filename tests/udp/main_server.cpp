#include "../../src/server.hpp"
#include <iostream>

int main(int argc, char** argv){

    UDPServer server("8448",1);

    int data[5];
    server.receive_data(data, 5*sizeof(int));
    
    std::cout << "Message:\n"; 
    for(int i=0; i<5; i++)
        std::cout << data[i] << " ";
    std::cout << "\n";

    std::cout << "Sending same data back\n";
    server.send_data(data, 5*sizeof(int));

    return 0;
}
