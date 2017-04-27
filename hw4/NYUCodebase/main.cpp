#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include "Matrix.h"
#include "ShaderProgram.h"
#define STB_IMAGE_IMPLEMENTATION
#include <math.h>
#include "Entity.h"
#include <iostream>
#include <string>
#include <vector>
#ifdef _WINDOWS
#define RESOURCE_FOLDER ""
#else
#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

//NOTES:
// 'A' and 'D' keys control p1(left), 'SPACE' to jump
// pickup the key and get to the door to make it 'unlock' and disappear


// 60 FPS (1.0f/60.0f)
#define FIXED_TIMESTEP 0.05f // 10 frames per second due to my CPU
#define MAX_TIMESTEPS 6

bool Debug=false;
bool Jumping=false;
bool hasKey=false;

const int WIDTH_PIXELS=640;
const int HEIGHT_PIXELS=360;
const float X_MIN=-3.55f;
const float X_MAX=3.55f;
const float Y_MIN=-2.0f;
const float Y_MAX=2.0f;
const float Z_MIN=-1.0f;
const float Z_MAX=1.0f;


SDL_Window* displayWindow;

void makeGrassLeft(Entity &object)
{
    object.addTexture("tiles_spritesheet.png");
    object.CalcTextureCoords(1024, 1024, 70, 70, 504, 648);
    object.eType=ENTITY_GRASS;
}

void makeGrassRight(Entity &object)
{
    object.addTexture("tiles_spritesheet.png");
    object.CalcTextureCoords(1024, 1024, 70, 70, 504, 504);
    object.eType=ENTITY_GRASS;
}

void makeGrassMid(Entity &object)
{
    object.addTexture("tiles_spritesheet.png");
    object.CalcTextureCoords(1024, 1024, 70, 70, 504, 576);
    object.eType=ENTITY_GRASS;
}

void makeHalfGrass(Entity &object)
{
    object.addTexture("tiles_spritesheet.png");
    object.CalcTextureCoords(1024, 1024, 70, 35, 576, 432);
    object.eType=ENTITY_GRASS;
}

std::vector<Entity> objects; //ALL THE ENTITIES

Matrix projectionMatrix;
Matrix modelMatrix;
Matrix viewMatrix;

void Setup() // CREATE EVERYTHING
{
    //PLAYER
    objects.push_back(Entity(0.5f, 0.5f, -1.0f, 1.5f));
    objects[0].Static=false;
    objects[0].eType=ENTITY_PLAYER;
    objects[0].addTexture("p3_stand.png");
    objects[0].CalcTextureCoords(66, 92, 66, 92, 0, 0);
    objects[0].gravity=-0.1f;
    objects[0].friction.x=0.25f;
    
    //DOOR
    objects.push_back(Entity(0.5, 0.5, 5.95f, -0.5f));
    objects[1].Static=true; //STATIC
    objects[1].eType=ENTITY_DOOR;
    objects[1].addTexture("tiles_spritesheet.png");
    objects[1].CalcTextureCoords(1024, 1024, 70, 69, 72, 1);
    
    //KEY
    objects.push_back(Entity(0.25, 0.25, 2.6f, 1.0f));
    objects[2].Static=false; //DYNAMIC
    objects[2].eType=ENTITY_KEY;
    objects[2].addTexture("keyRed.png");
    objects[2].CalcTextureCoords(70, 70, 60, 38, 5, 13);
    objects[2].gravity=-0.1f;
    objects[2].velocity.x=0.2f;
    
    //GRASS
    for (int i=0; i<24; i++)
    {
        objects.push_back(Entity(0.5f, 0.5f, (-3.55f+(i*0.5f)), -1.0f));
        switch (i)
        {
            case 0:
                makeGrassLeft(objects[(i+3)]);
                break;
            case 23:
                makeGrassRight(objects[(i+3)]);
                break;
            default:
                makeGrassMid(objects[(i+3)]);
                break;
        }
    }
    for (int i=0; i<5; i++) {
        objects.push_back(Entity(0.20f, 0.20f, 2.0f+(0.4f*i), 0.5f));
        makeHalfGrass(objects[(27+i)]);
    }
}

void Update(float elapsed) // UPDATE MOVEMENT AND POSTION
{
    for (int i = 0; i < objects.size(); i++)
    {
        if(!objects[i].Static)
        {
            objects[i].MoveY(elapsed);
            for (int j=0; j<objects.size(); j++)
            {
                if(i==j)
                {
                    continue;
                }
                else if(objects[i].collisionDetect(objects[j])) 
                {
                    if(objects[j].eType==ENTITY_GRASS)
                    {
                        if(objects[i].velocity.y<0)
                        {
                            objects[i].position.y+=(objects[i].getYPenetration(objects[j]))+0.01f;
                            objects[i].CollidedBottom=true;
                            Jumping=false;
                        }
                        else
                        {
                            objects[i].position.y-=(objects[i].getYPenetration(objects[j]))-0.1f;
                            objects[i].CollidedTop=true;
                        }
                        objects[i].velocity.y=0.0f;
                    }
                    else if(objects[j].eType==ENTITY_KEY)
                    {
                        hasKey=true;
                        objects[j].Active=false;
                    }
                    else if(objects[j].eType==ENTITY_PLAYER)
                    {
                        hasKey=true;
                        objects[i].Active=false;   
                    }
                    else if(objects[j].eType==ENTITY_DOOR && hasKey)
                    {
                        objects[j].Active=false;
                    }
                }
            }
            objects[i].MoveX(elapsed);
            for (int j=0; j<objects.size(); j++)
            {
                if(i==j)
                {
                    continue;
                }
                else if(objects[i].collisionDetect(objects[j])) 
                {
                    if(objects[j].eType==ENTITY_GRASS)
                    {
                        if(objects[i].velocity.x>0)
                        {
                            objects[i].position.x+=(objects[i].getXPenetration(objects[j]))+0.01f;
                            objects[i].CollidedRight=true;
                        }
                        else
                        {
                            objects[i].position.x-=(objects[i].getXPenetration(objects[j]))+0.01f;
                            objects[i].CollidedLeft=true;
                        }
                    }
                    else if(objects[j].eType==ENTITY_KEY)
                    {
                        hasKey=true;
                        objects[j].Active=false;
                    }
                    else if(objects[j].eType==ENTITY_PLAYER)
                    {
                        hasKey=true;
                        objects[i].Active=false;   
                    }
                    else if(objects[j].eType==ENTITY_DOOR && hasKey)
                    {
                        objects[j].Active=false;
                    }
                }
            }
            if (objects[i].eType==ENTITY_PLAYER)
            {
                if(objects[i].position.x>-3.0f && objects[i].position.x<6.0f)
                {
                    viewMatrix.setPosition(-objects[i].position.x, 0.0f, 0.0f);
                }
            }
            if (objects[i].eType==ENTITY_KEY)
            {
                if (objects[i].position.x<-2.0f || objects[i].position.x>4.0f)
                {
                    objects[i].velocity.x*=-1.0f;
                }
            }
        }
    }
}

void Render(ShaderProgram &program)
{
    for (int i=objects.size()-1; i>-1; i--) //SO THAT THE PLAYER IS RENDERED IN FRONT
    {
        objects[i].Draw(program);
    }
    // player.Draw(program);
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
            else if (event.type==SDL_KEYDOWN) // 'A' and 'D' keys control p1(left), 'SPACE' to jump
            {
                switch (event.key.keysym.sym)
                {
                    case SDLK_a:
                        objects[0].acceleration.x=-1.5f;
                        break;
                    case SDLK_d:
                        objects[0].acceleration.x=1.5f;
                        break;
                    case SDLK_SPACE:
                        if (!Jumping) // checking bottom collsion was too buggy
                        {
                            objects[0].velocity.y=2.5f;
                            Jumping=true;
                        }
                        break;
                    default:
                        break;
                }
            }
        }
        float ticks = (float)SDL_GetTicks()/1000.0f;
        float elapsed = ticks - lastFrameTicks;
        lastFrameTicks = ticks;
        
        float fixedElapsed = elapsed;
        if(fixedElapsed > FIXED_TIMESTEP * MAX_TIMESTEPS)
        {
            fixedElapsed = FIXED_TIMESTEP * MAX_TIMESTEPS;
        }
        while (fixedElapsed >= FIXED_TIMESTEP )
        {
            fixedElapsed -= FIXED_TIMESTEP;
            Update(FIXED_TIMESTEP);
        }
        Update(FIXED_TIMESTEP);
        
        glClear(GL_COLOR_BUFFER_BIT);
        
        program.setProjectionMatrix(projectionMatrix);
        program.setViewMatrix(viewMatrix);
        
        Render(program);
        
        SDL_GL_SwapWindow(displayWindow);
    }
    SDL_Quit();
    return 0;
}
