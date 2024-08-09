#include <iostream>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <cerrno>


class TCPInstance{
private: 
    int socket_fd;
    int verbose_level;

public:
    TCPInstance(int sock, int verbose=0):socket_fd(sock),verbose_level((verbose)){}

    ~TCPInstance(){
        close(socket_fd);
    }


    int send_data(const void *msg, int len){
        return send(socket_fd, msg, len, 0);
    }

    int receive_data(void *msg, int len){
        int n = recv(socket_fd, msg, len, 0);

        if(verbose_level>0)
            std::cout << "message received, length " << n << " bytes\n";

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

class TCPServer{
private:
    int socket_fd;
    int verbose_level = 0;

public:
    TCPServer(std::string port, int verbose=0){
        verbose_level = verbose;
        
        struct addrinfo hints, *res;

        // first, load up address structs with getaddrinfo():
        memset(&hints, 0, sizeof hints);
        hints.ai_family = AF_UNSPEC;     // use IPv4 or IPv6, whichever
        hints.ai_socktype = SOCK_STREAM; // TCP 
        hints.ai_flags = AI_PASSIVE;     // fill in my IP for me


        if(getaddrinfo(NULL, port.c_str(), &hints, &res)<0){
            perror("Failed setting address info");
            exit(EXIT_FAILURE);
        }

        // Create TCP server sockets
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

        // Bind socket to address
        if (bind(socket_fd, res->ai_addr, res->ai_addrlen) < 0) {
            perror("Bind failed");
            exit(EXIT_FAILURE);
        }

        // Set the listening conditions. The second argument is the backlog
        if(listen(socket_fd, 10)){
            perror("Listen failed");
            exit(EXIT_FAILURE);
        }

        if(verbose_level>0){
            std::cout << "Server ready to listen\n";
        }
    }

    ~TCPServer(){
        close(socket_fd);
    }

    int accept_connection(){



        // information about other side is not relevant for TCP
        struct sockaddr_storage their_addr;
        socklen_t addr_size = sizeof their_addr;


        if(verbose_level>0)
            std::cout << "Listening to connections\n";
        
        int new_socket_fd = accept(socket_fd, (struct sockaddr *)&their_addr, &addr_size); 

        if(verbose_level>0 && new_socket_fd>0)
            std::cout << "Accepted connection\n";

        // if(new_socket_fd>0)
        return new_socket_fd;
        // else 
        //     return NULL;
    }
};





class UDPServer{
private:
    int socket_fd;
    struct sockaddr_in cliaddr; 
    bool client_contacted = false;
    int verbose_level = 0;

public:
    UDPServer(std::string port, int verbose=0){
        verbose_level = verbose;
        struct addrinfo hints, *res;

        // Load up address structs with getaddrinfo()
        memset(&hints, 0, sizeof hints);
        hints.ai_family = AF_UNSPEC;    // use IPv4 or IPv6, whichever
        hints.ai_socktype = SOCK_DGRAM; // UDP
        hints.ai_flags = AI_PASSIVE;    // fill in my IP for me


        if(getaddrinfo(NULL, port.c_str(), &hints, &res)<0){
            perror("Failed setting address info");
            exit(EXIT_FAILURE);
        }

        // Create UDP server socket
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

        // Bind socket to address
        if (bind(socket_fd, res->ai_addr, res->ai_addrlen) < 0) {
            perror("Bind failed");
            exit(EXIT_FAILURE);
        }

        if(verbose_level>0){
            std::cout << "Server ready to listen\n";
        }

    }

    ~UDPServer(){
        close(socket_fd);
    }

    int send_data(const void *msg, int len){

        int n = -1;
        if(client_contacted)
        n = sendto(socket_fd, msg, len, MSG_CONFIRM, 
               (const struct sockaddr *) &cliaddr, sizeof(cliaddr));
        else{
            std::cout << "Client has not contacted yet. I don't know where to send data.\n";
        }
           
    
        return n;
    }

    int receive_data(void *msg, int len){

        if(verbose_level>0)
            std::cout << "Listening to message\n";
        
        socklen_t claddr_len;
        int n = recvfrom(socket_fd, msg, len,  
                MSG_WAITALL, ( struct sockaddr *) &cliaddr, 
                &claddr_len);

        if(n>0)
            client_contacted = true;
        if (n<0)  
            std::cerr << "recvfrom failed: " << strerror(errno) << std::endl;
        
        if(verbose_level>0 && n>0)
            std::cout << "message received, length " << n << " bytes\n";

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
