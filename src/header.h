// Prevent problems if this file is included twice
#ifndef HEADER_H
#define HEADER_H
#include <SDL2/SDL.h>
#define ui32 __uint32_t

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 420


typedef struct vector2 {
    ui32 x;
    ui32 y;
} vector2;

typedef struct Wall {
    vector2 w1;
    vector2 w2;

} Wall;

typedef struct Sprite {
    vector2 pos;
    float angle;
    SDL_Texture* texture;
} Sprite;
#endif  // HEADER_H
