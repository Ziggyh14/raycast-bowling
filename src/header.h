// Prevent problems if this file is included twice
#ifndef HEADER_H
#define HEADER_H
#include <SDL2/SDL.h>
#define ui32 __uint32_t

#define SCREEN_WIDTH 864
#define SCREEN_HEIGHT 486
#define TEXTURE_WIDTH 64
#define TEXTURE_HEIGHT 64

#define FRICTION_VAR 0.002;
#define BALL_MASS 1
#define PIN_MASS 0.1
#define HER_MASS 0.5;

#define max(a,b) ((a) > (b) ? (a) : (b))
#define min(a,b) ((a) < (b) ? (a) : (b))



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
    float vel;
    fvec2 dir;
    float mass;
} Sprite;
#endif  // HEADER_H
