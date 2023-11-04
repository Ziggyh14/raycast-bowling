#include <SDL2/SDL_render.h>
#include <SDL2/SDL_surface.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "header.h"
#define QUIT_CHECK if(SDL_QuitRequested()){break;}

struct {
    SDL_Window* window;
    SDL_Surface* surface;
    SDL_Renderer* rend;
    SDL_Texture* texture;
    ui32 pixels[SCREEN_WIDTH*SCREEN_HEIGHT];

    struct {
        vector2 pos;
        float angle, anglecos, anglesin;
        //int sector;
    } camera;

} state;



void verline(int x, int y1, int y2,ui32 color){
    for (int y = y1; y <= y2; y++) {
        state.pixels[(y * SCREEN_WIDTH) + x] = color;
    }
}

vector2 intersect_segs(vector2 w0, vector2 w1, vector2 r0, vector2 r1) {
    float d =
        ((w0.x - w1.x) * (r0.y - r1.y))
            - ((w0.y - w1.y) * (r0.x - r1.x));

    if (fabsf(d) < 0.000001f) { return (vector2) { NAN, NAN }; }

    float 
        t = (((w0.x - r0.x) * (r0.y - r1.y))
                - ((w0.y - r0.y) * (r0.x - r1.x))) / d,
        u = (((w0.x - r0.x) * (w0.y - w1.y))
                - ((w0.y - r0.y) * (w0.x - w1.x))) / d;
    return (t >= 0 && t <= 1 && u >= 0 && u <= 1) ?
        ((vector2) {
            w0.x + (t * (w1.x - w0.x)),
            w0.y + (t * (w1.y - w0.y)) })
        : ((vector2) { NAN, NAN });
}

int main(){
    Sprite* sprites = NULL;
    size_t numOfSprites = 1;

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

    state.camera.pos.x = 0;
    state.camera.pos.y = 0;

  double dirX = -1, dirY = 0; //initial direction vector
  double planeX = 0, planeY = 0.66; //the 2d raycaster version of camera plane

    Wall w = {{0,10},{10,11}};
    
    vector2 v = intersect_segs(w.w1,w.w2,(vector2){0,0},(vector2){500,1000});

    printf("(%d,%d)\n",v.x,v.y);

    int texture_pitch = 0;
    void* texture_pixels = NULL;
    if (SDL_LockTexture(state.texture, NULL, &texture_pixels, &texture_pitch) != 0) {
        SDL_Log("Unable to lock texture: %s", SDL_GetError());
    }
    else {
        memcpy(texture_pixels, state.pixels, texture_pitch * SCREEN_HEIGHT);
    }
    SDL_UnlockTexture(state.texture);

    sprites = malloc(numOfSprites * sizeof(Sprite));
    // Initialise sprites
    for(int i = 0; i < numOfSprites; i++) {
        sprites[i].pos = (vector2) {0, 0};
        sprites[i].angle = 0;
        char* imageFilePath = "res/ball.png";
        sprites[i].texture = IMG_LoadTexture(state.rend, imageFilePath);
    }
    

    while(1){
        QUIT_CHECK;


        // render on screen
        SDL_RenderClear(state.rend);
        SDL_RenderCopy(state.rend, state.texture, NULL, NULL);
        
        for (int i = 0; i < numOfSprites; i++) {
            int w, h;
            SDL_QueryTexture(sprites[i].texture, NULL, NULL, &w, &h);
            // Stretch it by 2x in both dimensions
            SDL_Rect destRect = (SDL_Rect) {0,0,w*2,h*2};
            SDL_RenderCopy(state.rend, sprites[i].texture, NULL, &destRect);
        }
        
        SDL_RenderPresent(state.rend);

    }

    SDL_DestroyTexture(state.texture);
    SDL_DestroyRenderer(state.rend);
    SDL_DestroyWindow(state.window);
    
    return 0;
}
