#include <SDL2/SDL_render.h>
#include <SDL2/SDL_surface.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "header.h"
#define QUIT_CHECK if(SDL_QuitRequested()){break;}

int worldMap[10][10]=
{
  {1,1,1,1,1,1,1,1,1,1},
  {1,0,0,0,0,0,1,0,0,1},
  {1,0,1,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,1,0,1},
  {1,1,0,0,0,0,0,0,0,1},
  {1,0,0,0,1,0,0,0,0,1},
  {1,1,1,1,1,1,1,1,1,1},
};

int* blank;

struct {
    SDL_Window* window;
    SDL_Surface* surface;
    SDL_Renderer* rend;
    SDL_Texture* texture;
    ui32 pixels[(SCREEN_WIDTH*SCREEN_HEIGHT)*sizeof(int)];

} state;

void verline(int x, int y1, int y2,ui32 color){
    for (int y = y1; y < y2; y++) {
        state.pixels[(y * SCREEN_WIDTH) + x] = color;
    }
}

void clearscreen(){
    int* temp[SCREEN_WIDTH*SCREEN_HEIGHT] = {0};
    memcpy(state.pixels,temp,SCREEN_WIDTH*SCREEN_HEIGHT*sizeof(int));
}

int isKeyDown(SDL_Event event){
    if(event.type==SDL_KEYDOWN)
        return 1;
    return 0;
}
SDL_Keycode getKeyPressed(SDL_Event event){
    return event.key.keysym.sym;
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
        
    state.rend = SDL_CreateRenderer(state.window,-1,SDL_RENDERER_PRESENTVSYNC );

    state.texture = SDL_CreateTexture(state.rend,SDL_PIXELFORMAT_ABGR8888,SDL_TEXTUREACCESS_STREAMING,
                                     SCREEN_WIDTH, SCREEN_HEIGHT);

    blank = calloc((SCREEN_WIDTH * SCREEN_HEIGHT),sizeof(int));

    double posX = 5, posY = 5;  //x and y start position
    double dirX = -1,dirY = 0; //initial direction vector
    double planeX = 0, planeY = 0.66; //the 2d raycaster version of camera plane

    double time = 0; //time of current frame
    double oldTime = 0; //time of previous frame

    sprites = malloc(numOfSprites * sizeof(Sprite));
    // Initialise sprites
    for(int i = 0; i < numOfSprites; i++) {
        sprites[i].pos = (vector2) {5, 4};
        sprites[i].angle = 0;
        char* imageFilePath = "res/ball.png";
        sprites[i].texture = IMG_LoadTexture(state.rend, imageFilePath);
    }
    

    while(1){

        QUIT_CHECK;

        for(int x = 0; x < SCREEN_WIDTH; x++)
        {
            double cameraX = 2 * x / (double) (SCREEN_WIDTH - 1); //x-coordinate in camera space
            double rayDirX = dirX + planeX * cameraX;
            double rayDirY = dirY + planeY * cameraX;

            //which box of the map we're in
            int mapX = (int) posX;
            int mapY = (int) posY;

            //length of ray from current position to next x or y-side
            double sideDistX;
            double sideDistY;

            //length of ray from one x or y-side to next x or y-side
            double deltaDistX = (rayDirX == 0) ? 1e30 : fabs(1 / rayDirX);
            double deltaDistY = (rayDirY == 0) ? 1e30 : fabs(1 / rayDirY);
            double perpWallDist;

            //what direction to step in x or y-direction (either +1 or -1)
            int stepX;
            int stepY;

            int hit = 0; //was there a wall hit?
            int side; //was a NS or a EW wall hit?

            if (rayDirX < 0)
            {
                stepX = -1;
                sideDistX = (posX - mapX) * deltaDistX;
            }
            else
            {
                stepX = 1;
                sideDistX = (mapX + 1.0 - posX) * deltaDistX;
            }
            if (rayDirY < 0)
            {
                stepY = -1;
                sideDistY = (posY - mapY) * deltaDistY;
            }
            else
            {
                stepY = 1;
                sideDistY = (mapY + 1.0 - posY) * deltaDistY;
            }

                  //perform DDA
            while (hit == 0)
            {
                //jump to next map square, either in x-direction, or in y-direction
                if (sideDistX < sideDistY)
                {
                sideDistX += deltaDistX;
                mapX += stepX;
                side = 0;
                }
                else
                {
                sideDistY += deltaDistY;
                mapY += stepY;
                side = 1;
                }
                //Check if ray has hit a wall
                if (worldMap[mapX][mapY] > 0) hit = 1;
            } 

            if(side == 0){
                perpWallDist = (sideDistX - deltaDistX);
            }else{
                perpWallDist = (sideDistY - deltaDistY);
            }

                  //Calculate height of line to draw on screen
            int lineHeight = (int)( SCREEN_HEIGHT/ perpWallDist);

            //calculate lowest and highest pixel to fill in current stripe
            int drawStart = -(lineHeight / 2) + SCREEN_HEIGHT / 2;
            if(drawStart < 0)
                drawStart = 0;
            int drawEnd = (lineHeight / 2) + (SCREEN_HEIGHT / 2);
            if(drawEnd >= SCREEN_HEIGHT)
                drawEnd = SCREEN_HEIGHT - 1;


            ui32 colour = 0x0000ff;
            if(side == 1)
                colour = 0x0000B8;
            //verline(x, 0, drawStart, 0xFF202020);
            verline(x, drawStart, drawEnd,colour);
            //verline(x, drawEnd, SCREEN_HEIGHT - 1, 0xFF505050);

        }
        //speed modifiers

        oldTime = time;
        time = SDL_GetTicks();
        double frameTime = (time - oldTime) / 1000.0; //frameTime is the time this frame has taken, in seconds
    //speed modifiers
        double moveSpeed = frameTime * 5.0; //the constant value is in squares/second
        double rotSpeed = frameTime * 3.0; //the constant value is in radians/second
       
        int texture_pitch = 0;
        void* texture_pixels = NULL;
        if (SDL_LockTexture(state.texture, NULL, &texture_pixels, &texture_pitch) != 0) {
        SDL_Log("Unable to lock texture: %s", SDL_GetError());
        }
        else {
        memcpy(texture_pixels, state.pixels, texture_pitch * SCREEN_HEIGHT);
        }
        SDL_UnlockTexture(state.texture);
        // render on screen
        SDL_RenderCopy(state.rend, state.texture, NULL, NULL);
        
        // Render sprites
        for (int i = 0; i < numOfSprites; i++) {
            int w, h;
            SDL_QueryTexture(sprites[i].texture, NULL, NULL, &w, &h);
            // Figure out where on the screen the texture should go and how big it should be
            // (relative to the camera)
            
            // Translate the position of the sprite to be relative to the camera
            double rx, ry;
            rx = sprites[i].pos.x - posX;
            ry = sprites[i].pos.y - posY;
            
            // Based on theory from https://lodev.org/cgtutor/raycasting3.html
            // Transform sprite with the inverse camera matrix
            
            // required for correct matrix multiplication
            double invDet = 1.0 / (planeX * dirY - dirX * planeY);
            
            double transformX = invDet * (dirY * rx - dirX * ry);
            double transformY = invDet * (-planeY * rx + planeX * ry);
            // This is actually the depth inside the screen - what Z is in 3D
            
            int spriteScreenX = (int) ((SCREEN_WIDTH / 2.0) * (1 + transformX / transformY));
            
            // Calculate height of the sprite on the screen
            int spriteHeight = abs((int) (SCREEN_HEIGHT / transformY));
            // Using transformY instead of the actual distance prevents fisheye
            
            // Calculate lowest and highest pixel to fill in
            int drawStartY = -spriteHeight / 2 + SCREEN_HEIGHT/2;
            if (drawStartY < 0) drawStartY = 0;
            int drawEndY = spriteHeight / 2 + SCREEN_HEIGHT/2;
            if (drawEndY >= SCREEN_HEIGHT) drawEndY = SCREEN_HEIGHT-1;
            
            // Calculate width of the sprite
            double spriteWidth = fabs( SCREEN_HEIGHT / transformY);
            int drawStartX = -spriteWidth / 2 + spriteScreenX;
            if (drawStartX < 0) drawStartX = 0;
            int drawEndX = spriteWidth / 2 + spriteScreenX;
            if (drawEndX >= SCREEN_WIDTH) drawEndX = SCREEN_WIDTH - 1;
            
            // Loop through every vertical stripe of the sprite on screen
            for (int stripe = drawStartX; stripe < drawEndX; stripe++) {
                double texX = ((stripe - (-spriteWidth / 2 + spriteScreenX)) * w / spriteWidth);
                // Skipped ZBuffer (so we have x-ray)
                if (transformY > 0 && stripe > 0 && stripe < SCREEN_WIDTH) {
                    SDL_Rect srcRect = {texX, 0, 1, h};
                    SDL_Rect destRect = {stripe, drawStartY, 1, drawEndY - drawStartY};
                    SDL_RenderCopy(state.rend, sprites[i].texture, &srcRect, &destRect);
                }
            }
        }
        
        SDL_RenderPresent(state.rend);
        clearscreen();

        SDL_Event e;
        while(SDL_PollEvent(&e)){
            QUIT_CHECK
            if(isKeyDown(e)){
                if(getKeyPressed(e) == SDLK_UP){
                    if(worldMap[(int)(posX + dirX * moveSpeed)][(int)posY] == 0) posX += dirX * moveSpeed;
                    if(worldMap[(int) posX][(int)(posY + dirY * moveSpeed)] == 0) posY += dirY * moveSpeed;
                }
                if(getKeyPressed(e) == SDLK_RIGHT){
                    //both camera direction and camera plane must be rotated
                    double oldDirX = dirX;
                    dirX = dirX * cos(-rotSpeed) - dirY * sin(-rotSpeed);
                    dirY = oldDirX * sin(-rotSpeed) + dirY * cos(-rotSpeed);
                    double oldPlaneX = planeX;
                    planeX = planeX * cos(-rotSpeed) - planeY * sin(-rotSpeed);
                    planeY = oldPlaneX * sin(-rotSpeed) + planeY * cos(-rotSpeed);
                }
                if(getKeyPressed(e) == SDLK_LEFT){
        
                    double oldDirX = dirX;
                    dirX = dirX * cos(rotSpeed) - dirY * sin(rotSpeed);
                    dirY = oldDirX * sin(rotSpeed) + dirY * cos(rotSpeed);
                    double oldPlaneX = planeX;
                    planeX = planeX * cos(rotSpeed) - planeY * sin(rotSpeed);
                    planeY = oldPlaneX * sin(rotSpeed) + planeY * cos(rotSpeed);
                }
                if(getKeyPressed(e) == SDLK_DOWN){
                    if(worldMap[(int)(posX - dirX * moveSpeed)][(int)(posY)] == 0) posX -= dirX * moveSpeed;
                    if(worldMap[(int)(posX)][(int)(posY - dirY * moveSpeed)] == 1) posY -= dirY * moveSpeed;
                }
            }   
        }
    }
    
    // Free up sprites
    for(int i = 0; i < numOfSprites; i++) {
        SDL_DestroyTexture(sprites[i].texture);
    }
    free(sprites);
    numOfSprites = 0;

    SDL_DestroyTexture(state.texture);
    SDL_DestroyRenderer(state.rend);
    SDL_DestroyWindow(state.window);
    
    return 0;
}
