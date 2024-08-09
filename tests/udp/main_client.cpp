#include "../../src/client.hpp"


int main(int argc, char** argv){

    UDPClient client("194.164.120.17", "8448", 1);

    int data[5]{1,2,1,2, 4};
    client.send_data(data, 5*sizeof(int));

    // client.send_data(data, 5*sizeof(int));

    int data_rec[5];
    client.receive_data(data_rec, 5*sizeof(int));
    
    std::cout << "Message:\n"; 
    for(int i=0; i<5; i++)
        std::cout << data_rec[i] << " ";
    std::cout << "\n";
    return 0;
}