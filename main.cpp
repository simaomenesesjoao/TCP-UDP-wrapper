#include "server.hpp"
#include "client.hpp"
#include <thread>
#include <iostream>

void functionServer(){

    TCPServer server("8081",1);
    TCPInstance connection = server.accept_connection();

    int data[5];
    connection.receive_data(data, 5*sizeof(int));
    
    std::cout << "Message:\n"; 
    for(int i=0; i<5; i++)
        std::cout << data[i] << " ";
    std::cout << "\n";
}


void functionClient(){
    TCPClient client("127.0.0.1", "8081", 1);
    client.continuous_connect();

    int data[5]{1,2,1,2, 4};
    client.send_data(data, 5*sizeof(int));
}


int main(int argc, char** argv){

    std::thread thread_server(&functionServer);
    std::thread thread_client(&functionClient);

    thread_server.join();
    thread_client.join();

    return 0;
}