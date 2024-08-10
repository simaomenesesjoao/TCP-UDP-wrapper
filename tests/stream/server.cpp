#include "../../src/server.hpp"
#include <iostream>
#include <cmath>

int main(int argc, char** argv){

    std::string portUDP = "8448";
    std::string portTCP = "8447";

    TCPServer TCP(portTCP, 1);
    int fd = TCP.accept_connection();

    MultiServer server(fd, portUDP);
    server.handshake();
    std::cout << "finished handshake\n";
    int dx = 100;
    int dy = 100;

    int header = 100;
    int tmax=200;
    int N=dx*dy;
    uint8_t buffer[N+header];
    
    // for(int i=0; i<nmes; i++){
        
    //     for(int j=0; j<N; j++){
    //         buffer[j] = j+i;
    //     }
    //     buffer[0] = x;
    //     buffer[1] = y;
    //     server.UDP.send_data(buffer, N);
    //     usleep(100*1000);
    // }

    int width = 400;
    int height = 400;
    for(int t=0; t<tmax; t++){
        std::cout << t << " " << std::flush;

        for(int xx=0; xx<width; xx+=dx){
            for(int yy=0; yy<height; yy+=dy){

                for(int i=0; i<dx; i++){
                    for(int j=0; j<dy; j++){
                        int x = xx + i;
                        int y = yy + j;
                        
                        int n = j*dx + i;

                        float pos0_x = sin(t*0.1)*0.5;
                        float pos0_y = cos(t*0.1)*0.5;
                        float pos_x = (x*2.0-width)/width - pos0_x;
                        float pos_y = (y*2.0-height)/height - pos0_y;
                        float z = cos(pos_x*10) + cos(pos_y*10);

                        
                        buffer[n+header] = (int)((2+z)*64);
                    }
                }
                buffer[0] = xx%256;
                buffer[1] = xx/256;
                buffer[2] = yy%256;
                buffer[3] = yy/256;
                server.TCP.send_data(buffer, N+header);
                usleep(10*1000);
            }
        }

        
    }
    std::cout << "\n";
   
    return 0;
}