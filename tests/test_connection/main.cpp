#include <cstdlib>
#include <iostream>
#include <memory>
#include <sys/socket.h>
#include <thread>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <vector>
#include <format>
#include <chrono>
#include <cerrno>
#include <cstring>
#include "../../src/server.hpp"
#include "../../src/client.hpp"


#define portUDP "8448"
#define portTCP "8447"
#define IP "127.0.0.1"

// https://linux.die.net/man/2/sendmsg

// ref: - https://medium.com/@adilrk/network-tools-19a12519737b
//      - https://medium.com/@zakharenko/how-to-simulate-network-failures-in-linux-b71ab585e86f
//
// network delay: 
//        100ms delay: sudo tc qdisc add dev lo root netem delay 100ms
//        distribution: sudo tc qdisc add dev lo root netem delay 100ms 40ms 50 distribution normal
// packet loss:
//        lose 50%, correlation 25%: sudo tc qdisc add dev lo root netem loss 50% 25%
// corruption: 
//        corrupt 50% (of packet content? or of packets?) sudo tc qdisc change dev lo root netem corrupt 50%
// combine delay and loss:
//        
// to remove any of these modifications, just replace "add" by "del" and run the modified command

using float_time_point = std::chrono::time_point<std::chrono::system_clock, std::chrono::duration<long, std::ratio<1,1000000000>>>;

float_time_point getCurrentTime() {
    float_time_point start = std::chrono::system_clock::now();
    return start;
}

struct DataPoint{
  float_time_point timestamp;
  int id;
  int checksum;
};

class Statistics{

public:
    std::vector<struct DataPoint> datapoints;

    void add_point(float_time_point timestamp, int id, int checksum){
        datapoints.push_back({timestamp,id, checksum});
    }

};

template <typename T>
void server_function(int type, Statistics* stat, bool* running, int size){


    TCPServer TCP(portTCP, 0);
    int fd = TCP.accept_connection();
    MultiServer server(fd, portUDP);
    server.handshake();

    
    T *buffer = new T[size];
    int count_til_timeout = 0;
    while(count_til_timeout<5){
        if(!*running) count_til_timeout++;

        int activity;
        if(type==0) activity = server.TCP.check_activity();
        if(type==1) activity = server.UDP.check_activity();

        if(activity==0){
            continue;
        } else {

            count_til_timeout = 0;

            int n=-1;
            if(type==0) n = server.TCP.receive_data(buffer, size*sizeof(T));
            if(type==1) n = server.UDP.receive_data(buffer, size*sizeof(T));
            if(n==0) break;
            float_time_point time = getCurrentTime();
            
            int id = (int)buffer[0];
            int sum=0;
            for(int i=0; i<size; i++) sum += (int)buffer[i];
            
            stat->add_point(time, id, sum);

            std::cout << "-";
            std::cout << "server: received " << n << " packet" << id << " with sum " << sum << "\n";

        }
    }

    delete[] buffer;
    std::cout << "\nLeft server\n" << std::flush;
}


template <typename T>
void client_function(int type, Statistics* stat, bool* running, int size, int delay_ms, int n_messages){

    MultiClient client(IP, portTCP);
    client.handshake();
    
    T *buffer = new T[size];
    for(int j=0; j<n_messages; j++){
        
        // Define the packet
        buffer[0] = j;
        for(int i=1; i<size; i++){
            buffer[i] = (i+5*j)%256;
        }

        int sum = 0;
        for(int i=0; i<size; i++) sum += buffer[i];

        std::cout << ".";

        int n;
        if(type==0) n = client.TCP.send_data(buffer, size*sizeof(T));
        if(type==1) n = client.UDP.send_data(buffer, size*sizeof(T));
        
        if (n == -1) {
            std::cout << "sendto failed\n";
            std::cerr << "Error code: " << errno << " (" << strerror(errno) << ")" << std::endl;
        }
        std::cout << "client: sending packet number "<< j << " " << n << "/" << size << " with sum " << sum << "\n";
        
        stat->add_point(getCurrentTime(), j, sum);
        usleep(delay_ms*1000);
    }

    delete[] buffer;
    std::cout << "\nLeft client\n" << std::flush;
    *running = false;
}



int main(int argc, char** argv){

    if(argc != 2){
        std::cout << "Please enter protocol as first argument. 0:TCP 1:UDP. Exiting.\n";
        exit(1);
    }

    int type = std::atoi(argv[1]);
    if(type == 0) std::cout << "TCP selected\n";
    if(type == 1) std::cout << "TCP selected\n";
;

    int size = 16000; // UDP has a maximum size, TCP doesn't 
    bool running = true;
    Statistics stat_client, stat_server;
    
    unsigned delay_client_ms = 1;
    unsigned n_messages = 100;

    // Get timing information and distribution from these threads through a variable passed by reference
    std::thread server_thread = std::thread(&server_function<int>, type, &stat_server, &running, size);
    std::thread client_thread = std::thread(&client_function<int>, type, &stat_client, &running, size, delay_client_ms, n_messages);
    
    client_thread.join();
    server_thread.join();

    // Process the statistics

    // Packets may be out of order
    // int count = 0;
    // for(auto &cli: stat_client.datapoints){
    //     bool found = false;
    //     for(auto &ser: stat_server.datapoints){
    //         if(cli.id == ser.id){
    //                 auto time_dif = std::chrono::duration_cast<std::chrono::milliseconds>(ser.timestamp - cli.timestamp);
    //                 bool checksum = ser.checksum == cli.checksum;
    //                 std::cout << std::format("{} {} {}\n", cli.id, time_dif, checksum);
    //             found = true;
    //             break;
    //         }
    //     }
    //     if(!found){
    //         std::cout << cli.id << "\n";
    //     }
    // }

    // auto start_time = stat_client.datapoints[0].timestamp;
    // auto end_time = stat_server.datapoints[stat_server.datapoints.size()-1].timestamp;
    // auto time_dif = std::chrono::duration_cast<std::chrono::milliseconds>(end_time-start_time);
    // std::cout << std::format("total time: {}\n", time_dif);

    
    return 0;
}