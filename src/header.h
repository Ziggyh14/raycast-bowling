
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