#include <iostream>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>

#define SERVER_IP "192.168.75.162" // Replace with server IP
#define TCP_PORT 8080
#define BUFFER_SIZE 1024

void handleUDPStream(int udp_port) {
    int udp_sock;
    struct sockaddr_in udp_serv_addr;
    char udp_buffer[BUFFER_SIZE];

    // Create UDP socket
    if ((udp_sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        std::cerr << "UDP socket creation failed" << std::endl;
        return;
    }

    memset(&udp_serv_addr, 0, sizeof(udp_serv_addr));
    udp_serv_addr.sin_family = AF_INET;
    udp_serv_addr.sin_port = htons(udp_port);
    udp_serv_addr.sin_addr.s_addr = INADDR_ANY;

    // Bind UDP socket to receive data
    if (bind(udp_sock, (struct sockaddr *)&udp_serv_addr, sizeof(udp_serv_addr)) < 0) {
        std::cerr << "UDP bind failed" << std::endl;
        close(udp_sock);
        return;
    }

    // Receive data over UDP
    while (true) {
        int len = recvfrom(udp_sock, udp_buffer, BUFFER_SIZE, 0, NULL, NULL);
        if (len > 0) {
            std::cout.write(udp_buffer, len);
            std::cout.flush();
        } else {
            break;
        }
    }

    std::cout << "Data received successfully over UDP!" << std::endl;
    close(udp_sock);
}

void handleTCPConnection(int tcp_sock) {
    int udp_port;

    // Receive UDP port from the server
    if (recv(tcp_sock, &udp_port, sizeof(udp_port), 0) < 0) {
        std::cerr << "Failed to receive UDP port" << std::endl;
        close(tcp_sock);
        return;
    }

    std::cout << "Received UDP port: " << udp_port << std::endl;

    // Start the UDP stream in a separate thread
    std::thread udp_thread(handleUDPStream, udp_port);

    // Keep the TCP connection open and listen for messages
    char buffer[BUFFER_SIZE];
    while (true) {
        int valread = recv(tcp_sock, buffer, BUFFER_SIZE, 0);
        if (valread > 0) {
            buffer[valread] = '\0';
            std::cout << "Received from server (TCP): " << buffer << std::endl;
        } else if (valread == 0) {
            std::cout << "Server disconnected." << std::endl;
            break;
        } else {
            perror("recv");
            break;
        }
    }

    udp_thread.join();
    close(tcp_sock);
}

int main() {
    int tcp_sock;
    struct sockaddr_in tcp_serv_addr;

    // Create TCP socket
    if ((tcp_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        std::cerr << "TCP socket creation error" << std::endl;
        return -1;
    }

    tcp_serv_addr.sin_family = AF_INET;
    tcp_serv_addr.sin_port = htons(TCP_PORT);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, SERVER_IP, &tcp_serv_addr.sin_addr) <= 0) {
        std::cerr << "Invalid server address" << std::endl;
        return -1;
    }

    // Connect to the server over TCP
    if (connect(tcp_sock, (struct sockaddr *)&tcp_serv_addr, sizeof(tcp_serv_addr)) < 0) {
        std::cerr << "TCP connection failed" << std::endl;
        return -1;
    }

    // Handle TCP connection in the main thread
    handleTCPConnection(tcp_sock);

    return 0;
}
