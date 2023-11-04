#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "header.h"
#include <SDL2/SDL.h>

#define QUIT_CHECK if(SDL_QuitRequested()){break;}

int worldMap[10][10]=
{
  {1,1,1,1,1,1,1,1,1,1},
  {1,0,0,0,0,0,1,0,0,1},
  {1,0,1,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,1,0,0,0,0,1},
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

    state.texture = SDL_CreateTexture(state.rend,SDL_PIXELFORMAT_ARGB8888,SDL_TEXTUREACCESS_STREAMING,
                                     SCREEN_WIDTH, SCREEN_HEIGHT);

    ui32 tex[TEXTURE_WIDTH*TEXTURE_HEIGHT];
    ui32 texfloor[TEXTURE_WIDTH*TEXTURE_HEIGHT];
    for(int x = 0; x<TEXTURE_WIDTH; x++){
        for(int y = 0; y<TEXTURE_HEIGHT;y++){
            tex[TEXTURE_WIDTH*y+x] = 65536 * 254 * (x != y && x != TEXTURE_WIDTH -y);
            texfloor[TEXTURE_WIDTH*y+x] = 0xF3F084;
        }
    }

    double posX = 5, posY = 5;  //x and y start position
    double dirX = -1,dirY = 0; //initial direction vector
    double planeX = 0, planeY = 0.66; //the 2d raycaster version of camera plane

    double time = 0; //time of current frame
    double oldTime = 0; //time of previous frame


    while(1){

        QUIT_CHECK;

        //FLOOR loop
        for(int y = 0; y<SCREEN_HEIGHT;y++){

            // rayDir for leftmost ray (x = 0) and rightmost ray (x = w)
            float rayDirX0 = dirX - planeX;
            float rayDirY0 = dirY - planeY;
            float rayDirX1 = dirX + planeX;
            float rayDirY1 = dirY + planeY;

            // horizon
            int p = y -SCREEN_HEIGHT /2;
            //vert pos of camera
            float posZ = 0.5 *SCREEN_HEIGHT;

            //Horzontal distance from the camera to the floor
            float rowDistance = posZ / p;
            // calculate the real world step vector we have to add for each x (parallel to camera plane)
            // adding step by step avoids multiplications with a weight in the inner loop
            float floorStepX = rowDistance * (rayDirX1 - rayDirX0) / SCREEN_WIDTH;
            float floorStepY = rowDistance * (rayDirY1 - rayDirY0) / SCREEN_WIDTH;

                    // real world coordinates of the leftmost column. This will be updated as we step to the right.
            float floorX = posX + rowDistance * rayDirX0;
            float floorY = posY + rowDistance * rayDirY0;

            for(int x = 0; x < SCREEN_WIDTH; ++x)
            {
                // the cell coord is simply got from the integer parts of floorX and floorY
                int cellX = (int)(floorX);
                int cellY = (int)(floorY);

                // get the texture coordinate from the fractional part
                int tx = (int)(TEXTURE_WIDTH * (floorX - cellX)) & (TEXTURE_WIDTH - 1);
                int ty = (int)(TEXTURE_HEIGHT * (floorY - cellY)) & (TEXTURE_HEIGHT - 1);

                floorX += floorStepX;
                floorY += floorStepY;

                // choose texture and draw the pixel
                //int floorTexture = 3;
                //int ceilingTexture = 6;
                Uint32 color;

                // floor
                color = texfloor[TEXTURE_WIDTH * ty + tx];
                color = (color >> 1) & 8355711; // make a bit darker
                state.pixels[(SCREEN_WIDTH*y)+x] = color;

                //ceiling (symmetrical, at screenHeight - y - 1 instead of y)
                color = texfloor[TEXTURE_WIDTH * ty + tx];
                color = (color >> 1) & 8355711; // make a bit darker
                state.pixels[(SCREEN_HEIGHT* (SCREEN_HEIGHT - y - 1))+ x] = color;
            }
        }

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

            //new code for textures 
            double wallX;
            if(side == 0)
                wallX = posY + perpWallDist * rayDirY;
            else
                wallX = posY +perpWallDist *rayDirX;
            
            wallX -= floor((wallX));

            int texX = (int)(wallX*(double)(TEXTURE_WIDTH));
            if(side == 0 && rayDirX > 0) texX = TEXTURE_WIDTH - texX - 1;
            if(side == 1 && rayDirY < 0) texX = TEXTURE_WIDTH - texX - 1;

            double step = 1.0 * TEXTURE_HEIGHT/lineHeight;

            double texPos = (drawStart - SCREEN_HEIGHT / 2 + lineHeight / 2) * step;
            for(int y = drawStart; y<drawEnd; y++)
            {
                // Cast the texture coordinate to integer, and mask with (texHeight - 1) in case of overflow
                int texY = (int)texPos & (TEXTURE_HEIGHT - 1);
                texPos += step;
                Uint32 color = tex[TEXTURE_HEIGHT * texY + texX];
                //make color darker for y-sides: R, G and B byte each divided through two with a "shift" and an "and"
                if(side == 1) color = (color >> 1) & 8355711;
                state.pixels[(SCREEN_WIDTH *y)+x] = color;
            }
            /*
            ui32 colour = 0x0000ff;
            if(side == 1)
                colour = 0x0000B8;
            //verline(x, 0, drawStart, 0xFF202020);
            verline(x, drawStart, drawEnd,colour);
            //verline(x, drawEnd, SCREEN_HEIGHT - 1, 0xFF505050);
            */
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
        //SDL_RenderClear(state.rend);
        SDL_RenderCopy(state.rend, state.texture, NULL, NULL);
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

    SDL_DestroyTexture(state.texture);
    SDL_DestroyRenderer(state.rend);
    SDL_DestroyWindow(state.window);
    
    return 0;
}

