#include "client.hpp"


int main(int argc, char** argv){

    TCPClient client("194.164.120.17", "8447", 1);
    int fd = client.continuous_connect();
    

    int data[5]{1,2,1,2, 4};
    client.send_data(data, 5*sizeof(int));
    return 0;
}