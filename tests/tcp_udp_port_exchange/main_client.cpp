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




    // int data[5]{1,2,1,2,4};
    // client.print_current_address();
    // client.send_data(data, 5*sizeof(int));

    // int data_rec[5];
    // client.print_current_address();
    // client.receive_data(data_rec, 5*sizeof(int));
    // std::cout << "Message received:"; 
    // for(int i=0; i<5; i++)std::cout << data_rec[i];
    // std::cout << "\n";

    // usleep(200*1000);
    // client.print_current_address();
    // client.send_data(data, 5*sizeof(int));
    // client.print_current_address();
    
    
    
    return 0;
}