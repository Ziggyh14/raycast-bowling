#include <SDL2/SDL_keyboard.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_surface.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include "sample_func.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "header.h"
#define QUIT_CHECK if(SDL_QuitRequested()){break;}

#define BOUNDARY_LINE 9

int worldMap[20][20]=
{
  {1,1,1,1,2,1,1,5,5,1,0,0,0,0,0,0,0,0,0,0},
  {3,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0},
  {3,0,1,0,0,0,0,0,0,1,4,4,4,4,4,4,4,4,4,4},
  {3,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7},
  {3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7},
  {3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7},
  {6,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7},
  {6,0,0,0,0,0,0,0,0,1,4,4,4,4,4,4,4,4,4,1},
  {3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7},
  {3,0,0,0,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7},
  {3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7},
  {2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7},
  {3,0,0,0,1,0,0,0,0,1,4,4,4,4,4,4,4,4,4,4},
  {3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7},
  {3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7},
  {3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7},
  {3,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7},
  {3,0,0,0,2,0,0,0,0,1,4,4,4,4,4,4,4,4,4,7},
  {3,0,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,0,0,7},
  {3,3,3,3,3,3,3,3,3,3,0,0,0,0,0,0,0,0,0,0},
};

int* blank;
TTF_Font* font;
SDL_Surface* msgSurface;
SDL_Texture* msgTexture;

long score;
int overTheLine;

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
int isKeyUp(SDL_Event event){
    if(event.type==SDL_KEYUP)
        return 1;
    return 0;
}
SDL_Keycode getKeyPressed(SDL_Event event){
    return event.key.keysym.sym;
}

double spriteDistance(Sprite s, double x, double y) {
    return sqrt(pow(s.pos.x - x, 2) + pow(s.pos.y - y, 2));
}

void load_texture(const char* file, WallTexture* dest){
    SDL_Surface* s = IMG_Load(file);
    if (s == NULL) {
        fprintf(stderr, "Failed to load %s, %s\n",file, SDL_GetError());
        exit(1);
    }
    SDL_Surface* s1 = SDL_ConvertSurfaceFormat(s, SDL_PIXELFORMAT_ARGB8888, 0);
    dest->width = s1->w;
    dest->height = s1->h;
    SDL_FreeSurface(s);
    if (s1 == NULL) {
        fprintf(stderr, "Failed to convert %s, %s\n",file, SDL_GetError());
        exit(1);
    }
    SDL_LockSurface(s1);
    dest->pixels = malloc((dest->width*dest->height)*sizeof(ui32));
    if (dest->pixels == NULL) {
        fprintf(stderr, "load_texture: Failed to allocate memory!");
        exit(1);
    }
    memcpy(dest->pixels, s1->pixels, (dest->width*dest->height)*sizeof(ui32));
    SDL_UnlockSurface(s1);
    SDL_FreeSurface(s1);
}


int main(){
    Sprite* sprites = NULL;
    size_t numOfSprites = 32;
    int* hitSprites;
    double zbuffer[SCREEN_WIDTH];

    int heldSprite = -1;
    
    create_Table();

    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)){
        printf("error initializing SDL: %s\n", SDL_GetError());
        return 1;
        }
    

    state.window =  SDL_CreateWindow(
            "Raycast Bowling",
            SDL_WINDOWPOS_CENTERED_DISPLAY(0),
            SDL_WINDOWPOS_CENTERED_DISPLAY(0),
            SCREEN_WIDTH,
            SCREEN_HEIGHT,
            SDL_WINDOW_ALLOW_HIGHDPI);
        
    state.rend = SDL_CreateRenderer(state.window,-1,SDL_RENDERER_PRESENTVSYNC );

    state.texture = SDL_CreateTexture(state.rend,SDL_PIXELFORMAT_ARGB8888,SDL_TEXTUREACCESS_STREAMING,
                                     SCREEN_WIDTH, SCREEN_HEIGHT);

    TTF_Init();
    font = TTF_OpenFont("res/vga_font.ttf", 16);
    if (font == NULL) {
        fprintf(stderr, "Failed to load font - so text might not be visible: %s\n", TTF_GetError());
    }
    // This message doesn't change so just render it at the start
    SDL_Color white = {255,255,255};
    SDL_Surface* msgSurface = TTF_RenderText_Solid(font, "Press E to pick up, hold & release space to throw", white);
    // Convert to surface (ugh)
    SDL_Texture* msgTexture = SDL_CreateTextureFromSurface(state.rend, msgSurface);
    
    init_Sample_Playback();

    

    size_t numOfTextures = 9;
    WallTexture textures[numOfTextures];

    load_texture("res/floor.png",&textures[0]);
    load_texture("res/carpet.png",&textures[1]);
    load_texture("res/wallblank.png",&textures[2]);
    load_texture("res/wallsign.png",&textures[3]);
    load_texture("res/bowling.png",&textures[4]);
    load_texture("res/wallgutter.png",&textures[5]);
    load_texture("res/wallrack.png",&textures[6]);
    load_texture("res/shoes.png",&textures[7]);
    load_texture("res/bowlingback.png",&textures[8]);
    
    double posX = 4.5, posY = 4.5;  //x and y start position
    double dirX = 1,dirY = 0; //initial direction vector
    double planeX = 0, planeY = -0.66 ;//the 2d raycaster version of camera plane

    float charge = 0;
    int win = 0;
    int delay = 0;

    double time = 0; //time of current frame
    double oldTime = 0; //time of previous frame

    sprites = malloc(numOfSprites * sizeof(Sprite));
    hitSprites = calloc(numOfSprites, sizeof(Sprite));
    hitSprites[0] = 1;
    
    double initSpritePositions[][2] = {
        {9.5, 5.5},

        {4.0, 9.0},
        {9.25, 18},
        {9.75, 18},
        {10.25, 18},
        {10.75, 18},
        {9.5, 17},
        {10.0, 17},
        {10.5, 17},
        {9.75, 16},
        {10.25, 16},
        {10.0, 15},

        {4.25, 18},
        {4.75, 18},
        {5.25, 18},
        {5.75, 18},
        {4.5, 17},
        {5.0, 17},
        {5.5, 17},
        {4.75, 16},
        {5.25, 16},
        {5.0, 15},

        {14.25, 18},
        {14.75, 18},
        {15.25, 18},
        {15.75, 18},
        {14.5, 17},
        {15.0, 17},
        {15.5, 17},
        {14.75, 16},
        {15.25, 16},
        {15.0, 15}

    };
    
    // Initialise sprites
    for(int i = 0; i < numOfSprites; i++) {
        sprites[i].origIndex = i;
        if (i < sizeof(initSpritePositions) / (sizeof(double)*2)) {
            sprites[i].pos.x = initSpritePositions[i][0];
            sprites[i].pos.y = initSpritePositions[i][1];
        } else {
            // Right in front of the player
            sprites[i].pos = (fvec2) {4 - (i/5.0),5};
        }
        sprites[i].angle = 0;
        sprites[i].dir = (fvec2){0,0};
        sprites[i].vel = 0;
        char* imageFilePath;
        float mass = 0;
        if (i == 0){
            imageFilePath = "res/ball.png";
            mass = BALL_MASS;
        }
        else if (i <= 11){ 
            imageFilePath = "res/her.png";
            mass = HER_MASS;
        }
        else {
            imageFilePath = "res/pin.png";
            mass = PIN_MASS;
        }
        sprites[i].mass = mass;
        sprites[i].texture = IMG_LoadTexture(state.rend, imageFilePath);
    }
    while(1){
        QUIT_CHECK;
        // Wait until the 2nd frame to slow down
        if (oldTime != 0) {
            double minFrameTime = 1000.0 / 60.0; /* 60 fps */
            double delta = time - oldTime;
            double timeToPass = minFrameTime - delta;
            if (timeToPass > 0) {
                SDL_Delay(timeToPass);
            }
        }

        //FLOOR loop
        for(int y = 0; y<SCREEN_HEIGHT;y++){

            // rayDir for leftmost ray (x = 0) and rightmost ray (x = w)
            float rayDirX0 = dirX - planeX;
            float rayDirY0 = dirY - planeY;
            float rayDirX1 = dirX + planeX;
            float rayDirY1 = dirY + planeY;

            // horizon
            int p = (y - SCREEN_HEIGHT /2);
            //vert pos of camera
            float posZ = 0.5 *SCREEN_HEIGHT;

            //Horzontal distance from the camera to the floor
            float rowDistance = posZ / p;
            // calculate the real world step vector we have to add for each x (parallel to camera plane)
            // adding step by step avoids multiplications with a weight in the inner loop
            float floorStepX = rowDistance * (rayDirX1 - rayDirX0) / SCREEN_WIDTH;
            float floorStepY = rowDistance * (rayDirY1 - rayDirY0) / SCREEN_WIDTH;

            // real world coordinates of the leftmost column. This will be updated as we step to the right.
            float floorX = posX + rowDistance* rayDirX0;
            float floorY = posY + rowDistance* rayDirY0;

            for(int x = 0; x < SCREEN_WIDTH; ++x)
            {
                // the cell coord is simply got from the integer parts of floorX and floorY
                int cellX = (int)(floorX);
                int cellY = (int)(floorY);

                // get the texture coordinate from the fractional part
                int tx = (int)(textures[0].width * (floorX-cellX)) & (textures[0].width-1);
                int ty = (int)(textures[0].height * (floorY-cellY)) &(textures[0].height-1);

                floorX += floorStepX;
                floorY += floorStepY;

                // choose texture and draw the pixel
                //int floorTexture = 3;
                //int ceilingTexture = 6;
                Uint32 color;

                // floor
                color = textures[0].pixels[textures[0].width * ty + tx];
                //color = (color >> 1) & 8355711; // make a bit darker
                state.pixels[SCREEN_WIDTH*y+x] = color;

                //ceiling (symmetrical, at screenHeight - y - 1 instead of y)
                color = textures[1].pixels[textures[1].width * ty + tx];
                state.pixels[(SCREEN_WIDTH* (SCREEN_HEIGHT - y - 1))+ x] = color;
            }
        }

        for(int x = 0; x < SCREEN_WIDTH; x++)  
        {
            double cameraX = 2 * (x / (double) (SCREEN_WIDTH - 1)) - 1; //x-coordinate in camera space
            double rayDirX = (dirX + planeX * cameraX) ;
            double rayDirY = (dirY + planeY * cameraX) ;

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
                wallX = posX + perpWallDist * rayDirX;
            
            
            wallX -= floor((wallX));

            int texX = (int)(wallX*(double)(textures[worldMap[mapX][mapY]+1].width));
            if(side == 0 && rayDirX > 0) texX = textures[worldMap[mapX][mapY]+1].width - texX - 1;
            if(side == 1 && rayDirY < 0) texX = textures[worldMap[mapX][mapY]+1].width - texX - 1;

            double step = 1.0 * TEXTURE_HEIGHT/lineHeight;

            double texPos = (drawStart - SCREEN_HEIGHT / 2.0 + lineHeight / 2.0) * step;
            for(int y = drawStart; y<drawEnd; y++)
            {
                // Cast the texture coordinate to integer, and mask with (texHeight - 1) in case of overflow
                int texY = (int)texPos & (textures[worldMap[mapX][mapY]+1].height - 1);
                texPos += step;
                Uint32 color = textures[worldMap[mapX][mapY]+1].pixels[textures[worldMap[mapX][mapY]+1].height * texY + texX];
                //make color darker for y-sides: R, G and B byte each divided through two with a "shift" and an "and"
                if(side == 1) color = (color >> 1) & 8355711;
                state.pixels[(SCREEN_WIDTH *y)+x] = color;
            }

            // Set the Z buffer for sprite casting
            zbuffer[x] = perpWallDist;
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
        
        // Reorder sprites in order of furthest to closest to the camera
        double spriteDists[numOfSprites];
        for(int i = 0; i < numOfSprites; i++) {
            if(sprites[i].vel > 0){
                sprites[i].pos = (fvec2) {sprites[i].pos.x + ((sprites[i].dir.x)* (sprites[i].vel *sprites[i].mass)),
                                          sprites[i].pos.y + ((sprites[i].dir.y)* (sprites[i].vel *sprites[i].mass))};
                sprites[i].vel-=FRICTION_VAR;
                max(0,sprites[i].vel);

                for (int j = 0; j < numOfSprites; j++) {
                    if(j!=i){
                        double d = spriteDistance(sprites[i],sprites[j].pos.x, sprites[j].pos.y);
                        if (d <0.4) {
                            if(sprites[i].mass == 1 && delay<=0){
                                play_Sample("res/strike.wav",0);
                                delay = 100;
                            }
                            float v = (sprites[i].mass * sprites[i].vel) + 
                            (sprites[j].mass * sprites[j].vel * sprites[i].mass) + sprites[i].mass;
                            printf("velcoty after collison of %d and %d: %f\n",i,j,v);
                            sprites[i].vel = v;
                            sprites[j].vel = v;
                            srand(SDL_GetTicks()<<j*217215);
                            if(sprites[i].mass > sprites[j].mass){
                                sprites[j].dir.x = sprites[i].dir.x + (0.5*((rand()%2)-1));
                                sprites[j].dir.y = sprites[i].dir.y;
                                sprites[j].vel = v+2;
                                sprites[i].vel = (v/2)-0.2;

                            }
                        }
                    }
                }
            }
        spriteDists[i] = spriteDistance(sprites[i], posX, posY);
        }
        if(delay>0)
            delay--;
        // Bubble sort the sprites
        int swapped = 1;
        while (swapped) {
            int numLeft = numOfSprites;
            swapped = 0;
            for (int x = 1; x < numLeft; x++) {
                if (spriteDists[x-1] <= spriteDists[x]) {
                    double tmpDist = spriteDists[x-1];
                    if (heldSprite == x-1) heldSprite = x;
                    else if (heldSprite == x) heldSprite = x-1;
                    Sprite tmpSprite = sprites[x-1];
                    spriteDists[x-1] = spriteDists[x];
                    sprites[x-1] = sprites[x];
                    spriteDists[x] = tmpDist;
                    sprites[x] = tmpSprite;
                }
            }
            numLeft--;
        }
       
        // Render sprites
        for (int i = 0; i < numOfSprites; i++) {
            // Hide held item
            if(heldSprite == i)
                continue;
            
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
            int spriteHeight = abs((int) (SCREEN_HEIGHT / transformY)) ;
            // Using transformY instead of the actual distance prevents fisheye
            
            // Calculate lowest and highest pixel to fill in
            int texStartY = 0, texEndY = h;
            int drawStartY = -spriteHeight / 2 + SCREEN_HEIGHT/2;
            if (drawStartY < 0) {
                texStartY = (0 - drawStartY) / (spriteHeight / h) ;
                drawStartY = 0;
            }
            int drawEndY = spriteHeight / 2 + SCREEN_HEIGHT/2;
            if (drawEndY >= SCREEN_HEIGHT) {
                texEndY = h - ((drawEndY - SCREEN_HEIGHT) / (spriteHeight / h));
                drawEndY = SCREEN_HEIGHT-1;
            }
            
            // Calculate width of the sprite
            int spriteWidth = fabs( SCREEN_HEIGHT / transformY) * ((double) w / h /* account for aspect ratio not being square */);
            int drawStartX = -spriteWidth / 2 + spriteScreenX;
            if (drawStartX < 0) drawStartX = 0;
            int drawEndX = spriteWidth / 2 + spriteScreenX;
            if (drawEndX >= SCREEN_WIDTH) drawEndX = SCREEN_WIDTH - 1;
            
            // Loop through every vertical stripe of the sprite on screen
            for (int stripe = drawStartX; stripe < drawEndX; stripe++) {
                double texX = ((stripe - (-spriteWidth / 2.0 + spriteScreenX)) * w / spriteWidth);
                if (transformY > 0 && stripe > 0 && stripe < SCREEN_WIDTH && zbuffer[stripe] >= transformY) {
                    SDL_Rect srcRect = {texX, texStartY, 1, texEndY - texStartY};
                    SDL_Rect destRect = {stripe, drawStartY, 1, drawEndY - drawStartY};
                    SDL_RenderCopy(state.rend, sprites[i].texture, &srcRect, &destRect);
                }
            }
        }
        
        if (heldSprite != -1) {
            // Show it in our “hand” if we're holding something
            int w, h;
            SDL_QueryTexture(sprites[heldSprite].texture, NULL, NULL, &w, &h);
            SDL_Rect srcRect = {w / 3, 0, w * 2 / 3, h * 2 / 3};
            SDL_Rect destRect = {0, SCREEN_HEIGHT - (SCREEN_WIDTH * 0.3), SCREEN_WIDTH * 0.3, SCREEN_WIDTH * 0.3};
            SDL_RenderCopy(state.rend, sprites[heldSprite].texture, &srcRect, &destRect);
        }
        
        // Render text
        SDL_Rect destRect = {0, 0, msgSurface->w, msgSurface->h};
        SDL_RenderCopy(state.rend, msgTexture, NULL, &destRect);
        
        // Render Score
        SDL_Color colour = {0};
        char scoreStr[64] = {0};
        if (posY >= BOUNDARY_LINE) {
            colour.r = 232; colour.g = 23; colour.b = 51;
            snprintf(scoreStr, 63, "You're over the line!");
        } else if (charge > 0.03) {
            colour.r = 128; colour.g = 255; colour.b = 128;
            snprintf(scoreStr, 63, "Charge: %.0f%%", min(100.0, charge / 0.5 * 100.0));
        } else {
            colour.r = 0; colour.g = 0; colour.b = 0;
            snprintf(scoreStr, 63, "Score: %li", score);
        }
        SDL_Surface* scoreSurface = TTF_RenderText_Solid(font, scoreStr, colour);
        // Convert to surface (ugh)
        SDL_Texture* scoreTexture = SDL_CreateTextureFromSurface(state.rend, scoreSurface);
        SDL_Rect scoreRect = {SCREEN_WIDTH - scoreSurface->w, SCREEN_HEIGHT - scoreSurface->h, scoreSurface->w, scoreSurface->h};
        SDL_RenderCopy(state.rend, scoreTexture, NULL, &scoreRect);
        SDL_FreeSurface(scoreSurface);
        SDL_DestroyTexture(scoreTexture);
        
        
        SDL_RenderPresent(state.rend);
        clearscreen();
        
        // Check which sprites the ball is hitting
        // Find the ball sprite
        Sprite* ballSprite = NULL;
        for(int i = 0; i < numOfSprites; i++) {
            if(sprites[i].origIndex == 0) {
                ballSprite = &sprites[i];
                if(ballSprite->pos.y > 20){
                   ballSprite->pos = (fvec2) {9.5,5.5};
                   ballSprite->vel = 0;
                }
                break;
            }
        }
        if (ballSprite == NULL) exit(2);
        
        for(int i = 0; i < numOfSprites; i++) {
            // Skip the ball
            if (sprites[i].origIndex == 0)
                continue;
            if( pow(sprites[i].pos.x - ballSprite->pos.x, 2) + pow(sprites[i].pos.y - ballSprite->pos.y, 2) < 0.5 ) {
                if(hitSprites[sprites[i].origIndex] == 0) {
                    hitSprites[sprites[i].origIndex] = 1;
                    score += 1;
                    if(score >= 10 && win==0){
                        win =1;
                        play_Sample("res/jazz.wav",0);
                    }
                }
            }
        }

        SDL_Event e;
        while(SDL_PollEvent(&e)){
            QUIT_CHECK
            
            if(isKeyDown(e)) {
                if(getKeyPressed(e) == SDLK_e && numOfSprites > 0) {
                    printf("E is pressed\n");
                    if (heldSprite == -1) {
                        // Not holding anything
                        // Get the closest sprite
                        double closestSpriteDist = INFINITY;
                        double closestSpriteIndex = 0;
                        for (int i = 0; i < numOfSprites; i++) {
                            double d = spriteDistance(sprites[i], posX, posY);
                            if (d < closestSpriteDist) {
                                closestSpriteDist = d;
                                closestSpriteIndex = i;
                            }
                        }
                        // If it's close enough, pick it up
                        if (closestSpriteDist < 1.5) {
                            heldSprite = closestSpriteIndex;
                        }
                    } else {
                        // Put it down
                        sprites[heldSprite].pos = (fvec2) {posX+(0.55*dirX),posY+(0.55*dirY)};
                        heldSprite = -1;
                        
                    }
                }
            }
            if(isKeyUp(e)){
                if(getKeyPressed(e) == SDLK_SPACE && numOfSprites >0){
                    if (heldSprite != -1 && posY < BOUNDARY_LINE) {
                        play_Sample("res/rolling.wav",0);
                        sprites[heldSprite].dir = (fvec2) {dirX,dirY};
                        sprites[heldSprite].vel = min(0.3,charge);
                        charge = 0;
                        sprites[heldSprite].pos = (fvec2) {posX+(0.2*dirX),posY+(0.2*dirY)};
                        heldSprite = -1;
                    }
                }
            }
        }
        const Uint8* keys;
        keys = SDL_GetKeyboardState(NULL);
        if(keys[SDL_SCANCODE_UP]){
            if(worldMap[(int)(posX + dirX * moveSpeed)][(int)posY] == 0) posX += (dirX * moveSpeed);
            if(worldMap[(int) posX][(int)(posY + dirY * moveSpeed)] == 0) posY += (dirY * moveSpeed);
        }
        if(keys[SDL_SCANCODE_RIGHT]){
            //both camera direction and camera plane must be rotated
            double oldDirX = dirX;
            dirX = dirX * cos(-rotSpeed) - dirY * sin(-rotSpeed);
            dirY = oldDirX * sin(-rotSpeed) + dirY * cos(-rotSpeed);
            double oldPlaneX = planeX;
            planeX = planeX * cos(-rotSpeed) - planeY * sin(-rotSpeed);
            planeY = oldPlaneX * sin(-rotSpeed) + planeY * cos(-rotSpeed);
        }
        if(keys[SDL_SCANCODE_LEFT]){
            double oldDirX = dirX;
            dirX = dirX * cos(rotSpeed) - dirY * sin(rotSpeed);
            dirY = oldDirX * sin(rotSpeed) + dirY * cos(rotSpeed);
            double oldPlaneX = planeX;
            planeX = planeX * cos(rotSpeed) - planeY * sin(rotSpeed);
            planeY = oldPlaneX * sin(rotSpeed) + planeY * cos(rotSpeed);
        }
        if(keys[SDL_SCANCODE_DOWN]){
            if(worldMap[(int)(posX - dirX * moveSpeed)][(int)(posY)] == 0) posX -= dirX * moveSpeed;
            if(worldMap[(int)(posX)][(int)(posY - dirY * moveSpeed)] == 0) posY -= dirY * moveSpeed;
        }
        if(keys[SDL_SCANCODE_SPACE] && numOfSprites > 0 && posY < BOUNDARY_LINE) {
            charge+=0.03;
        }
    }
    
    // Free up sprites
    for(int i = 0; i < numOfSprites; i++) {
        SDL_DestroyTexture(sprites[i].texture);
    }
    free(sprites);
    numOfSprites = 0;
    
    for(int i = 0; i < numOfTextures; i++) {
        free(textures[i].pixels);
    }

    close_Sample_Playback();
    SDL_DestroyTexture(state.texture);
    SDL_FreeSurface(msgSurface);
    SDL_DestroyTexture(msgTexture);
    SDL_DestroyRenderer(state.rend);
    SDL_DestroyWindow(state.window);
    
    return 0;

}
