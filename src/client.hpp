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


        // struct addrinfo *p;
        // char ipstr[INET6_ADDRSTRLEN];
        // for(p = res;p != NULL; p = p->ai_next) {
        //     void *addr;
        //     char *ipver;

        //     // get the pointer to the address itself,
        //     // different fields in IPv4 and IPv6:
        //     if (p->ai_family == AF_INET) { // IPv4
        //         struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
        //         addr = &(ipv4->sin_addr);
        //         ipver = "IPv4";
        //     } else { // IPv6
        //         struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)p->ai_addr;
        //         addr = &(ipv6->sin6_addr);
        //         ipver = "IPv6";
        //     }

        //     // convert the IP to a string and print it:
        //     inet_ntop(p->ai_family, addr, ipstr, sizeof ipstr);
        //     printf("  %s: %s\n", ipver, ipstr);
        // }

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

    int receive_data(void *msg, int len){
        return recv(socket_fd, msg, len, 0);
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

    int send_data(const void *msg, int len){

        int n = sendto(socket_fd, msg, len, MSG_CONFIRM, 
               servinfo->ai_addr, servinfo->ai_addrlen);
        
    
        return n;
    }

    int receive_data(void *msg, int len){

        // Receive
        socklen_t claddr_len;
        int n = recvfrom(socket_fd, msg, len,  
                MSG_WAITALL, NULL, NULL);


        return n;
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


// int main(){

//     // TCPClient client("127.0.0.1", "8081", 1);
//     // client.client_connect();

//     // int data[5]{1,2,1,2, 4};
//     // client.send_data(data, 5*sizeof(int));



//     UDPClient client("127.0.0.1", "8081", 1);

//     int data[5]{1,2,1,2, 4};
//     client.send_data(data, 5*sizeof(int));

//     return 0;
// }