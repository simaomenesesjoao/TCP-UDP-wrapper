#include <iostream>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>

#define TCP_PORT 8080
#define UDP_PORT 9090
#define BUFFER_SIZE 1024

void handleUDPStream(const std::string &client_ip) {
    int udp_server_fd;
    struct sockaddr_in udp_client_addr;
    char udp_buffer[BUFFER_SIZE];
    const char *data = "This is a large amount of data to be streamed over UDP...";
    int data_size = strlen(data);

    // Create UDP socket
    if ((udp_server_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("UDP socket creation failed");
        return;
    }

    memset(&udp_client_addr, 0, sizeof(udp_client_addr));
    udp_client_addr.sin_family = AF_INET;
    udp_client_addr.sin_port = htons(UDP_PORT);
    if (inet_pton(AF_INET, client_ip.c_str(), &udp_client_addr.sin_addr) <= 0) {
        perror("Invalid client IP address");
        close(udp_server_fd);
        return;
    }

    // Stream data over UDP
    while (true) {
        int sent_bytes = 0;
        while (sent_bytes < data_size) {
            int bytes_to_send = std::min(BUFFER_SIZE, data_size - sent_bytes);
            int bytes_sent = sendto(udp_server_fd, data + sent_bytes, bytes_to_send, 0, (struct sockaddr *)&udp_client_addr, sizeof(udp_client_addr));
            if (bytes_sent < 0) {
                perror("sendto");
                close(udp_server_fd);
                return;
            }
            sent_bytes += bytes_sent;
        }
        std::cout << "Data sent successfully over UDP!" << std::endl;

        sleep(1); // Adjust the sleep duration as needed
    }

    close(udp_server_fd);
}

void handleTCPConnection(int tcp_client_fd) {
    int udp_port = UDP_PORT;

    // Send UDP port information to the client
    if (send(tcp_client_fd, &udp_port, sizeof(udp_port), 0) < 0) {
        perror("send");
        close(tcp_client_fd);
        return;
    }

    std::cout << "Sent UDP port " << udp_port << " to client." << std::endl;

    // Keep the TCP connection open and listen for messages
    char buffer[BUFFER_SIZE];
    while (true) {
        int valread = recv(tcp_client_fd, buffer, BUFFER_SIZE, 0);
        if (valread > 0) {
            buffer[valread] = '\0';
            std::cout << "Received from client (TCP): " << buffer << std::endl;
        } else if (valread == 0) {
            std::cout << "Client disconnected." << std::endl;
            break;
        } else {
            perror("recv");
            break;
        }
    }

    close(tcp_client_fd);
}

int main() {
    int tcp_server_fd, tcp_client_fd;
    struct sockaddr_in tcp_address;
    int opt = 1;
    int addrlen = sizeof(tcp_address);
    char client_ip[INET_ADDRSTRLEN];

    // Creating TCP socket
    if ((tcp_server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("TCP socket failed");
        exit(EXIT_FAILURE);
    }

    // Setting TCP socket options
    if (setsockopt(tcp_server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    tcp_address.sin_family = AF_INET;
    tcp_address.sin_addr.s_addr = INADDR_ANY;
    tcp_address.sin_port = htons(TCP_PORT);

    // Binding TCP socket
    if (bind(tcp_server_fd, (struct sockaddr *)&tcp_address, sizeof(tcp_address)) < 0) {
        perror("TCP bind failed");
        close(tcp_server_fd);
        exit(EXIT_FAILURE);
    }

    // Start listening for TCP connections
    if (listen(tcp_server_fd, 3) < 0) {
        perror("listen");
        close(tcp_server_fd);
        exit(EXIT_FAILURE);
    }

    std::cout << "TCP server is listening on port " << TCP_PORT << std::endl;

    // Accept the TCP connection
    if ((tcp_client_fd = accept(tcp_server_fd, (struct sockaddr *)&tcp_address, (socklen_t*)&addrlen)) < 0) {
        perror("TCP accept");
        close(tcp_server_fd);
        exit(EXIT_FAILURE);
    }

    // Convert client IP address to string
    inet_ntop(AF_INET, &tcp_address.sin_addr, client_ip, sizeof(client_ip));
    std::cout << "connected ip is " << std::string(client_ip) << "\n";

    // Start the UDP stream in a separate thread
    std::thread udp_thread(handleUDPStream, std::string(client_ip));

    // Handle the TCP connection in the main thread
    handleTCPConnection(tcp_client_fd);

    // Join the UDP thread
    udp_thread.join();

    close(tcp_server_fd);
    return 0;
}
