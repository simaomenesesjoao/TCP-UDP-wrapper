#include "../../src/client.hpp"
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_stdinc.h>
#include <SDL2/SDL_surface.h>
#include <SDL2/SDL_video.h>
#include <iostream>
#include <SDL2/SDL.h>

class Graphics{
private:
    int width, height;
    SDL_Texture *texture;
    SDL_Renderer* renderer;
    SDL_Window* win;

public:
    Graphics(int width, int height):width(width),height(height){

        // Initialize SDL
        if (SDL_Init(SDL_INIT_EVERYTHING) != 0) 
            printf("error initializing SDL: %s\n", SDL_GetError()); 
        
        win = SDL_CreateWindow("UDP vs TCP stream", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, 0);;
        if (win == nullptr) 
            std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
        
        renderer = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);

        texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_BGRA8888, SDL_TEXTUREACCESS_STREAMING, width, height);
        SDL_SetTextureBlendMode( texture, SDL_BLENDMODE_BLEND );
        
    }
    
    void update(uint8_t* buffer_input){
        int pitch;
        int PIXEL_SIZE = 4;
        uint8_t *buf;
        SDL_LockTexture(texture, NULL,  (void **)&buf, &pitch);
        
        for(int i=0; i < height; i++){
            for(int j=0; j < width; j++){
                
                int n = (i*width + j);
                // std::cout << n << "\n";
                // ARGB
                buf[PIXEL_SIZE*n+0] = 255; // A
                buf[PIXEL_SIZE*n+1] = 0; // R
                buf[PIXEL_SIZE*n+2] = buffer_input[n]; // G
                buf[PIXEL_SIZE*n+3] = 0; // B
            }
        }

        SDL_UnlockTexture(texture);

        // Render it
        SDL_RenderClear(renderer);    
        SDL_Rect srcrect{0,0, width, height};    
        SDL_RenderCopy(renderer, texture, &srcrect, &srcrect);
        SDL_RenderPresent(renderer);
    }

    void update_rect(uint8_t* buffer_input, int x, int y, int dx, int dy){
        int pitch;
        int PIXEL_SIZE = 4;
        uint8_t *buf;
        SDL_LockTexture(texture, NULL,  (void **)&buf, &pitch);
        

        for(int i=y; i < y+dy; i++){
            for(int j=x; j < x+dx; j++){
                int m = (i-y)*dx + j-x;                
                int n = (i*width + j);
                
                
                buf[PIXEL_SIZE*n+0] = 255; // A
                buf[PIXEL_SIZE*n+1] = 0; // R
                buf[PIXEL_SIZE*n+2] = buffer_input[m]; // G
                buf[PIXEL_SIZE*n+3] = 0; // B
            }
        }

        SDL_UnlockTexture(texture);

        // Render it
        SDL_RenderClear(renderer);    
        SDL_Rect srcrect{0,0, width, height};    
        SDL_RenderCopy(renderer, texture, &srcrect, &srcrect);
        SDL_RenderPresent(renderer);
    }

    ~Graphics(){

        SDL_DestroyTexture(texture);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(win);
        SDL_Quit();	
    }

};



int main(int argc, char** argv){

    int height = 400;
    int width = 400;
    // int Npixels = height*width;
    Graphics graphics(width, height);

    // uint8_t graphics_buf[Npixels];
    // for(int i=0; i<Npixels; i++){
    //     graphics_buf[i] = i;
    // }

    // graphics.update(graphics_buf);
    // usleep(1*1000*1000);

    // int x,y,dx,dy;
    // x = 200;
    // y = 50;
    // dx = 100;
    // dy = 200;

    // uint8_t rec_buf[dx*dy];
    // for(int i=0; i<Npixels; i++){
    //     rec_buf[i] = 100;
    // }

    // graphics.update_rect(rec_buf, x, y, dx, dy);
    // usleep(1*1000*1000);


    std::string portTCP = "8447";
    // std::string IP = "194.164.120.17";

    std::string IP = "127.0.0.1";

    MultiClient client(IP, portTCP);
    client.handshake();
    std::cout << "finished handshake\n";

    int header = 100;
    int dx = 100;
    int dy = 100;
    int N = dx*dy;
    uint8_t buffer[N+header];

    int attempts = 0;
    while(attempts < 3){

        int activity = client.TCP.check_activity();
        if(activity>0){
            std::cout << "before receive \n" << std::flush;
            std::cout << "N:" << N << " N+h" << N+header << "\n" << std::flush;
            int n = client.TCP.receive_data(buffer, N+header);
            // std::cout << "after receive\n" << std::flush;
            int x = (int)buffer[0] + (int)buffer[1]*256;
            int y = (int)buffer[2] + (int)buffer[3]*256;
            std::cout << n << " " <<  x << " " << y << "\n";

            graphics.update_rect(buffer+header, x, y, dx, dy);


            

        } else {
            attempts ++;
        }
    }

    
    return 0;
}