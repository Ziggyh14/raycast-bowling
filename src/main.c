#include <stdio.h>
#include <stdlib.h>
#include "header.h"
#include <SDL2/SDL.h>

#define QUIT_CHECK if(SDL_QuitRequested()){break;}

struct state{
    SDL_Window* window;
    SDL_Surface* surface;
    SDL_Renderer* rend;
    SDL_Texture* texture;
    ui32 pixels[SCREEN_WIDTH *SCREEN_HEIGHT];

} state;

int main(){

    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)){
        printf("error initializing SDL: %s\n", SDL_GetError());
        return 1;
        }

    state.window =  SDL_CreateWindow(
            "game",
            SDL_WINDOWPOS_CENTERED_DISPLAY(0),
            SDL_WINDOWPOS_CENTERED_DISPLAY(0),
            SCREEN_WIDTH,
            SCREEN_HEIGHT,
            SDL_WINDOW_ALLOW_HIGHDPI);
        
    state.rend = SDL_CreateRenderer(state.window,-1,SDL_RENDERER_PRESENTVSYNC);

    state.texture = SDL_CreateTexture(state.rend,SDL_PIXELFORMAT_ABGR32,SDL_TEXTUREACCESS_STREAMING,
                                     SCREEN_WIDTH, SCREEN_HEIGHT);
                                    
    int texture_pitch = 0;
    void* texture_pixels = NULL;
    if (SDL_LockTexture(state.texture, NULL, &texture_pixels, &texture_pitch) != 0) {
        SDL_Log("Unable to lock texture: %s", SDL_GetError());
    }
    else {
        memcpy(texture_pixels, state.pixels, texture_pitch * SCREEN_HEIGHT);
    }
    SDL_UnlockTexture(state.texture);



    while(1){
        QUIT_CHECK;


        // render on screen
        SDL_RenderClear(state.rend);
        SDL_RenderCopy(state.rend, state.texture, NULL, NULL);
        SDL_RenderPresent(state.rend);

    }
    return 0;
}