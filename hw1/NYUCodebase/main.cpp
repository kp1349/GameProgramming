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


#ifdef _WINDOWS
#define RESOURCE_FOLDER ""
#else
#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

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

int main(int argc, char *argv[])
{
    SDL_Init(SDL_INIT_VIDEO);
    displayWindow = SDL_CreateWindow("My Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 360, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);
#ifdef _WINDOWS
    glewInit();
#endif
    
    //setup code ...
    glViewport(0, 0, 640, 360);

    GLuint crateTexture = LoadTexture(RESOURCE_FOLDER"crate2.png");
    GLuint dieTexture = LoadTexture(RESOURCE_FOLDER"dieRed1.png");
    GLuint houseTexture = LoadTexture(RESOURCE_FOLDER"pieceGreen_border07.png");
    
//    IMG_LoadTexture(<#SDL_Renderer *renderer#>, <#const char *file#>)
    
//    ShaderProgram program(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");
    
    ShaderProgram program(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");

//    ShaderProgram top(RESOURCE_FOLDER"vertex.glsl", RESOURCE_FOLDER"fragment.glsl");
    
    Matrix projectionMatrix;
    Matrix middleMatrix;
    Matrix staticMatrix;
    Matrix viewMatrix;

    projectionMatrix.setOrthoProjection(-3.55f, 3.55f, -2.0f, 2.0f, -1.0f, 1.0f);

    glUseProgram(program.programID);
//    glUseProgram(top.programID);
    
    int counter = 0;
    bool right = true;
    
    
    SDL_Event event;
    bool done = false;
    while (!done) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
                done = true;
            }
        }
        glClear(GL_COLOR_BUFFER_BIT);
        glClearColor(0.01f, 0.16f, 0.017f, 1.0f);
        
        if (counter>200){
            right=false;
        }
        else if (counter<-200){
            right=true;
        }
        
        if (right){
            middleMatrix.Translate(0.01f, 0.0f, 0.0f);
            counter++;
        }
        else {
            middleMatrix.Translate(-0.01f, 0.0f, 0.0f);
            counter--;
        }
        
        
        //loop code ...
        program.setModelMatrix(middleMatrix);
        program.setProjectionMatrix(projectionMatrix);
        program.setViewMatrix(viewMatrix);

//       	top.setModelMatrix(middleMatrix);
//       	top.setProjectionMatrix(projectionMatrix);
//       	top.setViewMatrix(viewMatrix);



        
        glBindTexture(GL_TEXTURE_2D, crateTexture);
        
        float middleVertices[] = {-0.5, -0.5, 0.5, -0.5, 0.5, 0.5,
            -0.5, -0.5, 0.5, 0.5, -0.5, 0.5};
        
        
        float topVertices[] = {-0.5, 1.0, 0.5, 1.0, 0.5, 2.0,
        	-0.5, 1.0, 0.5, 2.0, -0.5, 2.0};
        
        float bottomVertices[] = {-0.5, -1.0, 0.5, -1.0, 0.5, -2.0,
            -0.5, -1.0, 0.5, -2.0, -0.5, -2.0};


        glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, middleVertices);
//        glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, topVertices);
        glEnableVertexAttribArray(program.positionAttribute);
        

        float textureCoords[] = {0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0};
        glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, textureCoords);
        glEnableVertexAttribArray(program.texCoordAttribute);
        
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        glDisableVertexAttribArray(program.positionAttribute);
        glDisableVertexAttribArray(program.texCoordAttribute);

        
        program.setModelMatrix(staticMatrix);
        glBindTexture(GL_TEXTURE_2D, dieTexture);
        glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, topVertices);
        glEnableVertexAttribArray(program.positionAttribute);
        glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, textureCoords);
        glEnableVertexAttribArray(program.texCoordAttribute);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glDisableVertexAttribArray(program.positionAttribute);
        glDisableVertexAttribArray(program.texCoordAttribute);
        
//        program.setModelMatrix(staticMatrix);
        glBindTexture(GL_TEXTURE_2D, houseTexture);
        glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, bottomVertices);
        glEnableVertexAttribArray(program.positionAttribute);
        glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, textureCoords);
        glEnableVertexAttribArray(program.texCoordAttribute);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glDisableVertexAttribArray(program.positionAttribute);
        glDisableVertexAttribArray(program.texCoordAttribute);
        
        
        
        
        SDL_GL_SwapWindow(displayWindow);
    }
    
    SDL_Quit();
    return 0;
}
