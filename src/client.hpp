#include <iostream>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>

class TCPClient{
private:
    int socket_fd;
    int verbose_level=0;
    struct addrinfo hints, *res;

public:
    TCPClient(std::string ip, std::string port, int verbose=0){
        verbose_level = verbose;
        
        
        // first, load up address structs with getaddrinfo()
        memset(&hints, 0, sizeof hints);
        hints.ai_family = AF_INET;   // use IPv4 or IPv6, whichever
        hints.ai_socktype = SOCK_STREAM;

        if(getaddrinfo(ip.c_str(), port.c_str(), &hints, &res)<0){
            perror("Failed setting address info");
            exit(EXIT_FAILURE);
        }

        // Create TCP socket
        if ((socket_fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0) {
            perror("Socket failed");
            exit(EXIT_FAILURE);
        }

        int opt = 1;
        // Set socket options
        if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
            perror("setsockopt failed");
            exit(EXIT_FAILURE);
        }

        if(verbose_level>0){
            std::cout << "Client ready\n";
        }
    }

    ~TCPClient(){
        close(socket_fd);
    }

    int client_connect(){
        
        // Attempt to connect to the specified IP:port. If the server is not
        // connected, this will return -1
        int n = connect(socket_fd, res->ai_addr, res->ai_addrlen);
        
        if(n<0){
            std::cout << "Server not connected\n";
        } else if(verbose_level>0) {
            std::cout << "Connected to server\n";
        }

        return n;
    }

    int continuous_connect(int max_attempts = 10){
        
        // Attempt to connect to the specified IP:port. 
        int connected = -1;
        for(int attempt=0; attempt<max_attempts; attempt++){
            connected = connect(socket_fd, res->ai_addr, res->ai_addrlen);
            if(connected<0)
                usleep(200*1000);
            else
                break;
        }

        if(verbose_level>0) {
            if(connected<0)
                std::cout << "Failed to connect to server\n";
            else
                std::cout << "Connected to server\n";
        }

        return connected;
    }

    int send_data(const void *msg, int len){
        if(verbose_level>0)
            std::cout << "sending message\n";
        return send(socket_fd, msg, len, 0);
    }

    template <typename T>
    int receive_data(T *msg, int len){
        int total_received = 0;

        while(total_received<len){
            std::cout << "total_received" << total_received << "\n" << std::flush;
            std::cout << "len" << len << "\n" << std::flush;
            std::cout << "left" << len-total_received << "\n" << std::flush;
            // int n = recv(socket_fd, msg, len,0);
            int n = recv(socket_fd, msg+total_received, len-total_received, 
                MSG_WAITALL);
            std::cout << "n: " << n << "\n";
            total_received += n;
            
        }

        std::cout << "left recv data\n";

        return total_received;

        // return recv(socket_fd, msg, len, 0);

    }
    int check_activity(){
        fd_set readfds;
        struct timeval timeout;
        FD_ZERO(&readfds);
        FD_SET(socket_fd, &readfds);

        timeout.tv_usec = 0;
        timeout.tv_sec = 1;  

        int activity = select(socket_fd + 1, &readfds, nullptr, nullptr, &timeout);

        return activity;
    }

};



class UDPClient{
private:
    int socket_fd;

    int verbose_level=0;
    bool client_contacted = false;
    struct addrinfo *servinfo;

public:
    UDPClient(std::string ip, std::string port, int verbose = 0){
        verbose_level = verbose;

        struct addrinfo hints;
        // first, load up address structs with getaddrinfo():

        memset(&hints, 0, sizeof hints);
        hints.ai_family = AF_UNSPEC;   // use IPv4 or IPv6, whichever
        hints.ai_socktype = SOCK_DGRAM;
        hints.ai_flags = AI_PASSIVE;     // fill in my IP for me


        if(getaddrinfo(ip.c_str(), port.c_str(), &hints, &servinfo)<0){
            perror("Failed setting address info");
            exit(EXIT_FAILURE);
        }

        // Create TCP server sockets
        if ((socket_fd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol)) < 0) {
            perror("Socket failed");
            exit(EXIT_FAILURE);
        }

        int opt = 1;
        // Set socket options
        if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
            perror("setsockopt failed");
            exit(EXIT_FAILURE);
        }

        if(verbose_level>0)
            std::cout << "Client ready\n";

    }

    ~UDPClient(){
        close(socket_fd);
    }

    void print_current_address(){

        void *addr;
        char ipstr[INET6_ADDRSTRLEN];
        int port;
        if (servinfo->ai_family == AF_INET) { // IPv4
            struct sockaddr_in *ipv4 = (struct sockaddr_in *)servinfo->ai_addr;
            addr = &(ipv4->sin_addr);
            port = ntohs(ipv4->sin_port);
        } else { // IPv6
            struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)servinfo->ai_addr;
            addr = &(ipv6->sin6_addr);
            port = ntohs(ipv6->sin6_port);
        }
        inet_ntop(servinfo->ai_family, addr, ipstr, sizeof ipstr);
        std::cout << "IP: " << ipstr << " Port: " << port << std::endl;

    }
    int send_data(const void *msg, int len){


        int n = sendto(socket_fd, msg, len, MSG_CONFIRM, 
               servinfo->ai_addr, servinfo->ai_addrlen);

        if(verbose_level>0)
            std::cout << "Data sent: " << n << " bytes.\n";
    
        return n;
    }

    template <typename T>
    int receive_data(T *msg, int len){

        // Receive
        if(verbose_level>0)
            std::cout << "Listening to message\n";

        // int total_received = 0;
        // int n;
        // while(total_received<len){
        //     n = recvfrom(socket_fd, msg+total_received, len-total_received, 
        //         MSG_WAITALL, NULL, NULL);
        //     total_received += n;

        // }
        
        int total_received = recvfrom(socket_fd, msg, len, 
                MSG_WAITALL, NULL, NULL);

        if(verbose_level>0)
            std::cout << "message received, length " << total_received << " bytes\n";


        return total_received;
    }
    int check_activity(){
        fd_set readfds;
        struct timeval timeout;
        FD_ZERO(&readfds);
        FD_SET(socket_fd, &readfds);

        timeout.tv_usec = 0;
        timeout.tv_sec = 1;  

        int activity = select(socket_fd + 1, &readfds, nullptr, nullptr, &timeout);

        return activity;
    }

};



class MultiClient{
private:

    std::string getUDPport(){
        TCP.continuous_connect();

        char buffer[100];
        TCP.receive_data(buffer, 100);
        std::string response = std::string(buffer);
        std::cout << "Received: " << response << "\n";

        std::string portUDP = response.substr( response.find("=")+1, response.size());
        std::cout << "UDP port received: " << portUDP << "\n";
        return portUDP;
    }

public:

    TCPClient TCP;
    UDPClient UDP;

    MultiClient(std::string IP, std::string portTCP):TCP(IP,portTCP),UDP(IP, getUDPport()){

    }

    void handshake(){
        // std::string msg = "UDP OK";
        char buffer_c[100] = "UDP OK";
        // for(int i=0; i<msg.size()+1; i++) buffer_c[i] = msg.c_str()[i];

        UDP.send_data(buffer_c, 100);
        // UDP.send_data(msg.c_str(), msg.size()+1);

        // Receive acknowledgement that UDP packet was received
        char buffer[100];
        TCP.receive_data(buffer, 100);
        std::string response = std::string(buffer);
        std::cout << "Received: " << response << "\n";

        // Tell the server that the client is ready
        char buffer_d[100] = "Client ready";
        TCP.send_data(buffer_d, 100);
    }
    

};