// Prevent problems if this file is included twice
#ifndef HEADER_H
#define HEADER_H
#include <SDL2/SDL.h>
#define ui32 __uint32_t

#define SCREEN_WIDTH  640 //1020
#define SCREEN_HEIGHT 420 // 720
#define TEXTURE_WIDTH 64
#define TEXTURE_HEIGHT 64


typedef struct vector2 {
    ui32 x;
    ui32 y;
} vector2;

typedef struct fvec2 {
    double x;
    double y;
} fvec2;

typedef struct Wall {
    vector2 w1;
    vector2 w2;

} Wall;

typedef struct WallTexture {
    size_t width;
    size_t height;
    ui32* pixels;
} WallTexture;

typedef struct Sprite {
    fvec2 pos;
    // Used for changing the texture based on where it's facing (not yet implemented)
    float angle;
    SDL_Texture* texture;
} Sprite;
#endif  // HEADER_H
