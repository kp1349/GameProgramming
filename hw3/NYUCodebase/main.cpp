#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include "Matrix.h"
#include "ShaderProgram.h"
#define STB_IMAGE_IMPLEMENTATION
#include <math.h> //for sin and cos functions
#include "Entity.h"
#include <iostream>
#include <string>
#ifdef _WINDOWS
#define RESOURCE_FOLDER ""
#else
#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

//NOTES:
// 'A' and 'D' keys control p1(left), 'SPACE' to fire
// COLLISION DETECTION CAUSES GAME TO CRASH ON MY CPU


// 60 FPS (1.0f/60.0f)
#define FIXED_TIMESTEP 0.1f // 10 frames per second due to my CPU
#define MAX_TIMESTEPS 6

#define MAX_BULLETS 10
int bulletIndex = 0;



enum gameState {titleScreen, gameScreen};

bool Debug=false;

const int WIDTH_PIXELS=640;
const int HEIGHT_PIXELS=360;
const float X_MIN=-3.55f;
const float X_MAX=3.55f;
const float Y_MIN=-2.0f;
const float Y_MAX=2.0f;
const float Z_MIN=-1.0f;
const float Z_MAX=1.0f;

SDL_Window* displayWindow;

//VARIABLES
std::vector<std::vector<Entity>> enemies;
Entity player1 = Entity(1.0f, 1.0f, 0.0f, -1.5f); // player 1
std::vector<Entity> title;
std::vector<Entity> prompt;
gameState STATE = titleScreen;
//std::vector<Entity> projectiles;
Entity bullets[MAX_BULLETS];

void shootBullet() {
    bullets[bulletIndex].Active=true;
    bullets[bulletIndex].direction.y=0.5f;
    bullets[bulletIndex].position.x=player1.position.x;
    bullets[bulletIndex].position.y=player1.top+bullets[bulletIndex].length.y;
    bulletIndex++;
    if(bulletIndex > MAX_BULLETS-1) {
        bulletIndex = 0;
    }
}

void Setup() // CREATE EVERYTHING
{
    // SET UP PLAYER 1
    player1.addTexture("sheet.png");
    player1.CalcTextureCoords(1024, 1024, 98, 75, 325, 0);
    player1.changeSize(0.25f);
    player1.friction.x=0.5f;
    
    
    for (int i=0; i<4; i++) {   //CREATING ENEMIES
        std::vector<Entity> enemyRow;
        for (int j=0; j<6; j++) {
            enemyRow.push_back(Entity(1.0f, 1.0f, (-2.5f)+((float)j), 1.5f-((float)i*0.5f)));
            enemyRow[j].addTexture("sheet.png");
            switch (i) {
                case 0:
                    enemyRow[j].CalcTextureCoords(1024, 1024, 93, 84, 425, 468); // enemy 1 image
                    break;
                case 1:
                    enemyRow[j].CalcTextureCoords(1024, 1024, 104, 84, 143, 293); // enemy 2 image
                    break;
                case 2:
                    enemyRow[j].CalcTextureCoords(1024, 1024, 103, 84, 222, 0); // enemy 3 image
                    break;
                case 3:
                    enemyRow[j].CalcTextureCoords(1024, 1024, 97, 84, 421, 814); // enemy 4 image
                default:
                    break;
            }
            enemyRow[j].direction.x=0.02f;
            enemyRow[j].changeSize(0.25f);
        }
        enemies.push_back(enemyRow);
    }
    
    //CREATING TEXT
    Vector2 x;
    x.x=-2.5f;
    x.y=1.0f;
    title = Entity::printText(x, "SPACE INVADERS", 0.5f);//TITLE
    x.y=-1.0f;
    x.x=-2.0f;
    prompt = Entity::printText(x, "PRESS ENTER TO START", 0.25f);//PROMPT
    
    //BULETS
    for(int i=0; i < MAX_BULLETS; i++)
    {
        bullets[i] = Entity(0.2f, 1.0f, 0.0f, 0.0f); // laser
        bullets[i].addTexture("sheet.png");
        bullets[i].CalcTextureCoords(1024, 1024, 9, 54, 858, 230);
        bullets[i].changeSize(0.5f);
        bullets[i].Active=false;
    }
    
}

void Update(float elapsed) // UPDATE MOVEMENT AND POSTION
{
    bool invert = false;
    switch (STATE) {
        case gameScreen:
            if((enemies[0][0].direction.x>0 && enemies[0][0].position.x>-2.0f) || (enemies[0][0].direction.x<0 && enemies[0][0].position.x<-3.0f))
            {
                invert=true;
            }
            for (int i=0; i<enemies.size(); i++)
            {
                for (int j=0; j<enemies[i].size(); j++) // RUN ON EACH ENEMY
                {
                    for(int i=0; i < MAX_BULLETS; i++)
                    {
                        if (bullets[i].Active && bullets[i].collisionDetect(enemies[i][j])) // CAUSES THE GAME TO CRASH
                        {
                            bullets[i].Active=false;
                            enemies[i][j].Active=false;
                        }
                    }
                    if (invert){enemies[i][j].direction.x*=-1.0f;}
                    enemies[i][j].Move(elapsed);    //MOVE
                }
            }
            invert=false;
            for(int i=0; i < MAX_BULLETS; i++)
            {
                if (bullets[i].Active)
                {
                    bullets[i].Move(elapsed);
                }
            }
            player1.Move(elapsed);
            break;
        case titleScreen:
            break;
        default:
            break;
    }
}

void Render(ShaderProgram &program)
{
    switch (STATE) {
        case gameScreen:
            player1.Draw(program);
            for (int i=0; i<enemies.size(); i++)
            {
                for (int j=0; j<enemies[i].size(); j++) // RUN ON EACH ENEMY
                {
                    if (enemies[i][j].Active)
                    {
                        enemies[i][j].Draw(program);    //DRAW
                    }
                }
            }
            for(int i=0; i < MAX_BULLETS; i++)
            {
                if (bullets[i].Active)
                {
                    bullets[i].Draw(program);
                }
            }
            break;
        case titleScreen:
            for (int i=0; i<title.size(); i++) {
                title[i].Draw(program);     //TOP LINE OF TEXT
            }
            for (int i=0; i<prompt.size(); i++) {
                prompt[i].Draw(program);    //BOTTOM PROMPT
            }
            break;
        default:
            break;
    }
}


int main(int argc, char *argv[])
{
    SDL_Init(SDL_INIT_VIDEO);
    displayWindow = SDL_CreateWindow("My Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH_PIXELS, HEIGHT_PIXELS, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);
#ifdef _WINDOWS
    glewInit();
#endif
    
    //setup code ...
    glViewport(0, 0, WIDTH_PIXELS, HEIGHT_PIXELS);
    
    ShaderProgram program(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");
    
    Matrix projectionMatrix;
    Matrix modelMatrix;
    Matrix viewMatrix;
    
    projectionMatrix.setOrthoProjection(X_MIN, X_MAX, Y_MIN, Y_MAX, Z_MIN, Z_MAX);
    
    glUseProgram(program.programID);
    
    Setup();
    
    float lastFrameTicks = 0.0f;
    SDL_Event event;
    bool done = false;
    
    while (!done) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE)
            {
                done = true;
            }
            else if (event.type==SDL_KEYDOWN) // 'A' and 'D' keys control p1(left), 'SPACE' to fire
            {
                switch (event.key.keysym.sym)
                {
                    case SDLK_RETURN:
                        if(STATE==titleScreen){STATE=gameScreen;}
                        break;
                    case SDLK_ESCAPE:
                        if(STATE==gameScreen){STATE=titleScreen;}
                        break;
                    case SDLK_a:
                        if(STATE==gameScreen){player1.acceleration.x=-2.0f;}
                        break;
                    case SDLK_d:
                        if(STATE==gameScreen){player1.acceleration.x=2.0f;}
                        break;
                    case SDLK_SPACE:
                        if(STATE==gameScreen){shootBullet();}
                        break;
                    default:
                        break;
                }
            }
            
            float ticks = (float)SDL_GetTicks()/1000.0f;
            float elapsed = ticks - lastFrameTicks;
            lastFrameTicks = ticks;
            
            float fixedElapsed = elapsed;
            if(fixedElapsed > FIXED_TIMESTEP * MAX_TIMESTEPS) {
                fixedElapsed = FIXED_TIMESTEP * MAX_TIMESTEPS;
            }
            while (fixedElapsed >= FIXED_TIMESTEP ) {
                fixedElapsed -= FIXED_TIMESTEP;
                Update(FIXED_TIMESTEP);
            }
            Update(FIXED_TIMESTEP);
            
            glClear(GL_COLOR_BUFFER_BIT);
            
            program.setProjectionMatrix(projectionMatrix);
            program.setViewMatrix(viewMatrix);
            
            Render(program);
//            DrawText(program, texture, "hi", 1.0f, 0.0f); // DECIDED NOT TO USE IT, AS IT CAN ONLY BE ON ONE LINE
            
            
            
            SDL_GL_SwapWindow(displayWindow);
        }
    }
    
    SDL_Quit();
    return 0;
}
