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

// 60 FPS (1.0f/60.0f)
#define FIXED_TIMESTEP 0.01666f
#define MAX_TIMESTEPS 6

bool Debug=false;
bool Jumping=false;
bool hasKey=false;

const int WIDTH_PIXELS=640*1;
const int HEIGHT_PIXELS=360*1;
const float X_MIN=-3.55f;
const float X_MAX=3.55f;
const float Y_MIN=-2.0f;
const float Y_MAX=2.0f;
const float Z_MIN=-1.0f;
const float Z_MAX=1.0f;


SDL_Window* displayWindow;

std::vector<Entity> objects; //ALL THE ENTITIES

Matrix projectionMatrix;
Matrix modelMatrix;
Matrix viewMatrix;

bool outOfUD(Entity object)
{
    return ((object.position.y + object.length.y) > Y_MAX) || ((object.position.y - object.length.y) < Y_MIN);
}

bool outOfLR(Entity object)
{
    return ((object.position.x + object.length.x) > X_MAX) || ((object.position.x - object.length.x) < X_MIN);
}

///*
bool penetrationSort(const Vector &p1, const Vector &p2) {
    return p1.length() < p2.length();
}

bool testSATSeparationForEdge(float edgeX, float edgeY, const std::vector<Vector> &points1, const std::vector<Vector> &points2, Vector &penetration) {
    float normalX = -edgeY;
    float normalY = edgeX;
    float len = sqrtf(normalX*normalX + normalY*normalY);
    normalX /= len;
    normalY /= len;
    
    std::vector<float> e1Projected;
    std::vector<float> e2Projected;
    
    for(int i=0; i < points1.size(); i++) {
        e1Projected.push_back(points1[i].x * normalX + points1[i].y * normalY);
    }
    for(int i=0; i < points2.size(); i++) {
        e2Projected.push_back(points2[i].x * normalX + points2[i].y * normalY);
    }
    
    std::sort(e1Projected.begin(), e1Projected.end());
    std::sort(e2Projected.begin(), e2Projected.end());
    
    float e1Min = e1Projected[0];
    float e1Max = e1Projected[e1Projected.size()-1];
    float e2Min = e2Projected[0];
    float e2Max = e2Projected[e2Projected.size()-1];
    
    float e1Width = fabs(e1Max-e1Min);
    float e2Width = fabs(e2Max-e2Min);
    float e1Center = e1Min + (e1Width/2.0);
    float e2Center = e2Min + (e2Width/2.0);
    float dist = fabs(e1Center-e2Center);
    float p = dist - ((e1Width+e2Width)/2.0);
    
    if(p >= 0) {
        return false;
    }
    
    float penetrationMin1 = e1Max - e2Min;
    float penetrationMin2 = e2Max - e1Min;
    
    float penetrationAmount = penetrationMin1;
    if(penetrationMin2 < penetrationAmount) {
        penetrationAmount = penetrationMin2;
    }
    
    penetration.x = normalX * penetrationAmount;
    penetration.y = normalY * penetrationAmount;
    
    return true;
}

bool checkSATCollision(const std::vector<Vector> &e1Points, const std::vector<Vector> &e2Points, Vector &penetration) {
    std::vector<Vector> penetrations;
    for(int i=0; i < e1Points.size(); i++) {
        float edgeX, edgeY;
        
        if(i == e1Points.size()-1) {
            edgeX = e1Points[0].x - e1Points[i].x;
            edgeY = e1Points[0].y - e1Points[i].y;
        } else {
            edgeX = e1Points[i+1].x - e1Points[i].x;
            edgeY = e1Points[i+1].y - e1Points[i].y;
        }
        Vector penetration;
        bool result = testSATSeparationForEdge(edgeX, edgeY, e1Points, e2Points, penetration);
        if(!result) {
            return false;
        }
        penetrations.push_back(penetration);
    }
    for(int i=0; i < e2Points.size(); i++) {
        float edgeX, edgeY;
        
        if(i == e2Points.size()-1) {
            edgeX = e2Points[0].x - e2Points[i].x;
            edgeY = e2Points[0].y - e2Points[i].y;
        } else {
            edgeX = e2Points[i+1].x - e2Points[i].x;
            edgeY = e2Points[i+1].y - e2Points[i].y;
        }
        Vector penetration;
        bool result = testSATSeparationForEdge(edgeX, edgeY, e1Points, e2Points, penetration);
        
        if(!result) {
            return false;
        }
        penetrations.push_back(penetration);
    }
    
    
    std::sort(penetrations.begin(), penetrations.end(), penetrationSort);
    penetration = penetrations[0];
    
    Vector e1Center;
    for(int i=0; i < e1Points.size(); i++) {
        e1Center.x += e1Points[i].x;
        e1Center.y += e1Points[i].y;
    }
    e1Center.x /= (float)e1Points.size();
    e1Center.y /= (float)e1Points.size();
    
    Vector e2Center;
    for(int i=0; i < e2Points.size(); i++) {
        e2Center.x += e2Points[i].x;
        e2Center.y += e2Points[i].y;
    }
    e2Center.x /= (float)e2Points.size();
    e2Center.y /= (float)e2Points.size();
    
    Vector ba;
    ba.x = e1Center.x - e2Center.x;
    ba.y = e1Center.y - e2Center.y;
    
    if( (penetration.x * ba.x) + (penetration.y * ba.y) < 0.0f) {
        penetration.x *= -1.0f;
        penetration.y *= -1.0f;
    }
    
    return true;
}
//*/



void Setup() // CREATE EVERYTHING
{
    objects.push_back(Entity()); // BLOCK 1
    objects[0].position.x=-3.0f;
    objects[0].position.y=0.5f;
    objects[0].velocity.x=0.5f;
    objects[0].velocity.y=-0.5f;
    objects.push_back(Entity()); // BLOCK 2
    objects[1].position.y=-1.5f;
    objects[1].position.x=1.0f;
    objects[1].velocity.x=-0.5f;
    objects[1].velocity.y=0.5f;
    objects.push_back(Entity()); // BLOCK 3
    objects[2].position.x=3.0f;
    objects[2].position.y=-0.5f;
    objects[2].velocity.x=0.5f;
    objects[2].velocity.y=-0.5f;
    objects.push_back(Entity()); // EXTRA BLOCK
    for (int i = 0; i < objects.size(); i++)
    {
        // TO CREATE ANY KIND OF RECTANGLE
        objects[i].scale.x=1.0f;
        objects[i].scale.y=2.0f;
    }
    
}

void Update(float elapsed) // UPDATE MOVEMENT AND POSTION
{
    for (int i=0; i<objects.size(); i++) {
        objects[i].Move(elapsed);
        
        if(outOfLR(objects[i])){objects[i].velocity.x*=-1.0f;} // LEFT/RIGHT
        if(outOfUD(objects[i])){objects[i].velocity.y*=-1.0f;} // UP/DOWN
        objects[i].rotationAngle+=(3.14159265/128); // ROTATION
        objects[i].setVertices(); // SETUP FOR SAT COLLISION CHECK
    }
    
    Vector penetration = Vector();

    for (int i = 0; i < objects.size(); i++)
    {
        for (int j = 0; j < objects.size(); j++)
        {
            if (i!=j) // DON'T CHECK AGAINST SAME ENTITY
            {
                if(checkSATCollision(objects[i].vertices, objects[j].vertices, penetration))
                {
                    if (objects[i].position.x < objects[j].position.x)
                    {
                        objects[i].position.x-=0.01f;
                        objects[j].position.x+=0.01f;
                        objects[i].velocity.x-=cosf(penetration.x);
                        objects[j].velocity.x+=cosf(penetration.x);
                    }
                    else{
                        objects[j].position.x-=0.01f;
                        objects[i].position.x+=0.01f;
                        objects[j].velocity.x-=cosf(penetration.x);
                        objects[i].velocity.x+=cosf(penetration.x);
                    }
                    if (objects[i].position.y < objects[j].position.y)
                    {
                        objects[i].position.y-=0.01f;
                        objects[j].position.y+=0.01f;
                        objects[i].velocity.y-=sinf(penetration.y);
                        objects[j].velocity.y+=sinf(penetration.y);
                    }
                    else{
                        objects[j].position.y-=0.01f;
                        objects[i].position.y+=0.01f;
                        objects[j].velocity.y-=sinf(penetration.y);
                        objects[i].velocity.y+=sinf(penetration.y);
                    }
                }
            }
        }
        
    }
}

void Render(ShaderProgram &program)
{
    for (int i=objects.size()-1; i>-1; i--)
    {
        objects[i].Draw(program);
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
    
    ShaderProgram program(RESOURCE_FOLDER"vertex.glsl", RESOURCE_FOLDER"fragment.glsl");
    
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
