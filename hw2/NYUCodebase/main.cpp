#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include "Matrix.h"
#include "ShaderProgram.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <assert.h>
#include <math.h> //for sin and cos functions
#include <iostream>
#include <string>



#ifdef _WINDOWS
#define RESOURCE_FOLDER ""
#else
#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

bool Debug=false;

int WIDTH_PIXELS=640;
int HEIGHT_PIXELS=360;
float X_MIN=-3.55f;
float X_MAX=3.55f;
float Y_MIN=-2.0f;
float Y_MAX=2.0f;
float Z_MIN=-1.0f;
float Z_MAX=1.0f;

SDL_Window* displayWindow;

GLuint LoadTexture(const char *filePath) {
    int w,h,comp;
    unsigned char* image = stbi_load(filePath, &w, &h, &comp, STBI_rgb_alpha);
    if(image == NULL) {
        std::cout << "Unable to load image. Make sure the path is correct\n";
        assert(false);
    }
    GLuint retTexture;
    glGenTextures(1, &retTexture);
    glBindTexture(GL_TEXTURE_2D, retTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    stbi_image_free(image);
    return retTexture;
}

class Entity{
public:
    Matrix modelMatrix;
    float top, bottom, left, right, width, height; // dimensions
    int textureID;
    float position_x, position_y, speed, direction_x, direction_y, rotation0; // movement
    
    Entity(float Top, float Bottom, float Left, float Right) // Constructor
    {
        top=Top;
        bottom=Bottom;
        left=Left;
        right=Right;
        
        height=(top-bottom)/2;
        width=fabs(right-left)/2;
        position_x=(right+left)/2;
        position_y=(top+bottom)/2;
        speed=1.0f;
        direction_x=0.0f;
        direction_y=0.0f;
    }
    void Draw(ShaderProgram &program)   // SDL code after setting vertex
    {
        glEnableVertexAttribArray(program.positionAttribute);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glDisableVertexAttribArray(program.positionAttribute);
    };
    void Move(float ticks)  // Allows the object to move
    {
        float moveRight=direction_x*speed*ticks;
        float moveUp=direction_y*speed*ticks;
        position_y+=moveUp;
        position_x+=moveRight;
        top+=moveUp;
        bottom+=moveUp;
        right+=moveRight;
        left+=moveRight;
        modelMatrix.Translate(moveRight, moveUp, 0.0f);
        if(Debug){std::cout<<"new position x: "+std::to_string(position_x)+"\nnew position y: "+std::to_string(position_y)+'\n';}
    };
    bool collisionDetect(Entity &object)    // detect a collision and change direction accordingly
    {
        return (top>object.bottom && bottom<object.top && left<object.right && right>object.left);
    };
    bool outOfScreen()
    {
        return (position_x>X_MAX || position_x<X_MIN);
    };
    void setPosition(float newPositionX, float newPositionY)
    {
        position_x=newPositionX;
        position_y=newPositionY;
        top=position_y+height;
        bottom=position_y-height;
        left=position_x-width;
        right=position_x+width;
        modelMatrix.setPosition(position_x, position_y, 0.0f);
    };
};

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
    glViewport(0, 0, 640, 360);
    
    ShaderProgram program(RESOURCE_FOLDER"vertex.glsl", RESOURCE_FOLDER"fragment.glsl");
    
    Matrix projectionMatrix;
    Matrix modelMatrix;
    Matrix viewMatrix;
    
    projectionMatrix.setOrthoProjection(X_MIN, X_MAX, Y_MIN, Y_MAX, Z_MIN, Z_MAX);

    glUseProgram(program.programID);
    
    Entity topWall(2.0f, 1.8f, -3.55f, 3.55f);
    float topWallVertices[] =
    {
        topWall.right, topWall.top,
        topWall.left, topWall.top,
        topWall.left, topWall.bottom,
        topWall.right, topWall.top,
        topWall.left, topWall.bottom,
        topWall.right, topWall.bottom
    };
    
    Entity bottomWall(-1.8f, -2.0f, -3.55f, 3.55f);
    float bottomWallVertices[] =
    {
        bottomWall.right, bottomWall.top,
        bottomWall.left, bottomWall.top,
        bottomWall.left, bottomWall.bottom,
        bottomWall.right, bottomWall.top,
        bottomWall.left, bottomWall.bottom,
        bottomWall.right, bottomWall.bottom
    };
    
    Entity ball(0.1f, -0.1f, -0.1f, 0.1f);
    float ballVertices[] =
    {
        ball.right, ball.top,
        ball.left, ball.top,
        ball.left, ball.bottom,
        ball.right, ball.top,
        ball.left, ball.bottom,
        ball.right, ball.bottom
    };
    ball.direction_x=1.0f;
    ball.direction_y=1.0f;
    
    Entity player1(0.5f, -0.5f, -3.45f, -3.25f); //left paddle
    float player1Vertices[] =
    {
        player1.right, player1.top,
        player1.left, player1.top,
        player1.left, player1.bottom,
        player1.right, player1.top,
        player1.left, player1.bottom,
        player1.right, player1.bottom
    };
    
    Entity player2(0.5f, -0.5f, 3.25f, 3.45f); //left paddle
    float player2Vertices[] =
    {
        player2.right, player2.top,
        player2.left, player2.top,
        player2.left, player2.bottom,
        player2.right, player2.top,
        player2.left, player2.bottom,
        player2.right, player2.bottom
    };
    
    float lastFrameTicks = 0.0f;
    bool outOfScreen=false;
    
//    float old_y=0.0f;
    
    SDL_Event event;
    bool done = false;
    while (!done) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE)
            {
                done = true;
            }
//            else if (event.type==SDL_MOUSEMOTION)
//            {
//                float unitX = (((float)event.motion.x / 640.0f) * 7.1f ) - 3.55f;
//                float unitY = (((float)(360-event.motion.y) / 360.0f) * 4.0f ) - 2.0f;
//                std::cout<<"event.motion.y: "<<std::to_string(unitY)<<'\n';
//                float change = unitY-old_y;
//                player1.speed=change;
//                old_y=unitY;
//            }
            else if (event.type==SDL_KEYDOWN) // 'w' and 's' keys control p1(left), 'up' and 'down' keys control p2(right)
            {
                switch (event.key.keysym.sym)
                {
                    case SDLK_w:
                        player1.direction_y=7.0f;
                        break;
                    case SDLK_s:
                        player1.direction_y=-7.0f;
                        break;
                    case SDLK_UP:
                        player2.direction_y=7.0f;
                        break;
                    case SDLK_DOWN:
                        player2.direction_y=-7.0f;
                        break;
                    default:
                        break;
                }
    
            }
        }
        float ticks = (float)SDL_GetTicks()/1000.0f;
        float elapsed = ticks - lastFrameTicks;
        lastFrameTicks = ticks;
        
        glClear(GL_COLOR_BUFFER_BIT);
        if(outOfScreen) // Changes the color of the screen if the ball goes out of the screen
        {
            glClearColor(0.1f, 0.0f, 1.0f, 1.0f);
            ball.setPosition(0.0f, 0.0f);
        }
        
        program.setProjectionMatrix(projectionMatrix);
        program.setViewMatrix(viewMatrix);
        
        program.setModelMatrix(topWall.modelMatrix);
        glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, topWallVertices);
        topWall.Draw(program);
        
        program.setModelMatrix(bottomWall.modelMatrix);
        glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, bottomWallVertices);
        bottomWall.Draw(program);
        
        ball.Move(elapsed);
        if (ball.collisionDetect(topWall)){ball.direction_y*=-1;}
        if (ball.collisionDetect(bottomWall)){ball.direction_y*=-1;}
        if (ball.collisionDetect(player1)){ball.direction_x*=-1;}
        if (ball.collisionDetect(player2)){ball.direction_x*=-1;}
        outOfScreen=ball.outOfScreen();
        program.setModelMatrix(ball.modelMatrix);
        glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, ballVertices);
        ball.Draw(program);
        
        player1.Move(elapsed);
        program.setModelMatrix(player1.modelMatrix);
        glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, player1Vertices);
        ball.Draw(program);
        player1.direction_y=0.0f;
        
        player2.Move(elapsed);
        program.setModelMatrix(player2.modelMatrix);
        glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, player2Vertices);
        ball.Draw(program);
        player2.direction_y=0.0f;
        
        
        SDL_GL_SwapWindow(displayWindow);
    }
    
    SDL_Quit();
    return 0;
}
