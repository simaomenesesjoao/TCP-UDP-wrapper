#include "../../src/client.hpp"


int main(int argc, char** argv){

    std::string portTCP = "8447";
    std::string IP = "194.164.120.17";
    
    MultiClient client(IP, portTCP);
    client.handshake();
    

    
    
    return 0;
}