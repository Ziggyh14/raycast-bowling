
#define ui32 __uint32_t

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720


typedef struct vector2 {
    ui32 x;
    ui32 y;
} vector2;

typedef struct Wall {
    ui32 x1;
    ui32 y1;
    ui32 x2;
    ui32 y2;
    float slope;

} Wall;