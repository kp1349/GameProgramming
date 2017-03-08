//
//  Entity.cpp
//  NYUCodebase
//
//  Created by Koushik Paul on 2/26/17.
//  Copyright © 2017 Ivan Safrin. All rights reserved.
//

#include "Entity.h"
#include <assert.h>
#ifdef _WINDOWS
#define RESOURCE_FOLDER ""
#else
#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif
#include <string>

GLuint Entity::LoadTexture(const char *filePath)
{
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

void Entity::addTexture(std::string name)
{
    texture=LoadTexture((RESOURCE_FOLDER+name).c_str());
}

Entity::Entity(float Width, float Height, float x, float y)
{
    modelMatrix = Matrix();
    length.x=(float)Width/2;
    length.y=(float)Height/2;
    position.x=x;
    position.y=y;
    
    textureTop=0.0f;
    textureBottom=1.0f;
    textureLeft=0.0f;
    textureRight=1.0f;
    
    direction.x=0.0f;
    direction.y=0.0f;
    acceleration.x=0.0f;
    acceleration.y=0.0f;
    gravity=2.0f;
    friction.x=0.0f;
    friction.y=0.0f;
    
    Debug=true;
    Active=true;

    updateAll();
}

void Entity::updateAll()
{
    top=position.y+length.y;
    bottom=position.y-length.y;
    right=position.x+length.x;
    left=position.x-length.x;
}

void Entity::Draw(ShaderProgram &program)
{
    program.setModelMatrix(modelMatrix);
    glBindTexture(GL_TEXTURE_2D, texture);
    
    float vertices[] =
    {
        right, top,
        left, top,
        left, bottom,
        right, top,
        left, bottom,
        right, bottom
    };
    
    float textureVertices[] =
    {
        textureRight, textureTop,
        textureLeft, textureTop,
        textureLeft, textureBottom,
        textureRight, textureTop,
        textureLeft, textureBottom,
        textureRight, textureBottom
    };
    
//    float textureVertices[] =
//    {
//        spriteSheet.Left, spriteSheet.Top,
//        spriteSheet.Right, spriteSheet.Top,
//        spriteSheet.Right, spriteSheet.Bottom,
//        spriteSheet.Left, spriteSheet.Top,
//        spriteSheet.Right, spriteSheet.Bottom,
//        spriteSheet.Left, spriteSheet.Bottom
//    };
    glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(program.positionAttribute);
    glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, textureVertices);
    glEnableVertexAttribArray(program.texCoordAttribute);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glDisableVertexAttribArray(program.positionAttribute);
    glDisableVertexAttribArray(program.texCoordAttribute);
}

void Entity::CalcTextureCoords(int totalWidth, int totalHeight, int Width, int Height, int Left, int Top, int LRoffset, int UDoffset)
{
    textureTop=(float)(Top+UDoffset)/totalHeight;
//    std::cout<<textureTop<<std::endl;
    textureBottom=(float)(Top+Height-UDoffset)/totalHeight;
//    std::cout<<textureBottom<<std::endl;
    textureLeft=(float)(Left+LRoffset)/totalWidth;
//    std::cout<<textureLeft<<std::endl;
    textureRight=(float)(Left+Width-LRoffset)/totalWidth;
//    std::cout<<textureRight<<std::endl;
    Width-=2*LRoffset;
    Height-=2*UDoffset;
    float ratio=(float)Width/Height;
    length.x*=ratio;
    updateAll();
}

void Entity::changeSize(float changeByThisMuch)
{
    length.x*=changeByThisMuch;
    length.y*=changeByThisMuch;
    updateAll();
}

void Entity::Move(float ticks)
{
    direction.x = lerp(direction.x, 0.0f, ticks*friction.x);
    direction.y = lerp(direction.y, 0.0f, ticks*friction.x);
    direction.x += acceleration.x*ticks;
    direction.y += acceleration.y*ticks;
    acceleration.x=0;
    acceleration.x=0;
    float moveRight=direction.x*ticks;
    float moveUp=direction.y*ticks;
    position.x+=moveRight;
    position.y+=moveUp;
    updateAll();
//    if(Debug){std::cout<<"new position x: "+std::to_string(position.x)+"\nnew position y: "+std::to_string(position.y)+'\n';}
}

std::vector<Entity> Entity::printText(Vector2 Position, std::string line, float changeSize)
{
    std::vector<Entity> entities;
    int LRoffset=8;
    int UDoffset=5;
    int dimensions=512;
    int width=32;
    int height=32;
    float increment = (changeSize*1.0f);
    increment-=increment*((float)LRoffset/width);
//    float start = -3.55f+(increment/2);
    float current = Position.x;
    for (int i=0; i<line.length(); i++) {
        entities.push_back(Entity(1.0f, 1.0f, current, Position.y));
        current+=increment;
        entities[i].addTexture("font1.png");
//        std::cout<<line[i]<<std::endl;
        float x,y;
        switch (line[i]) {
            case 'A':
                x=1;
                y=4;
                break;
            case 'B':
                x=2;
                y=4;
                break;
            case 'C':
                x=3;
                y=4;
                break;
            case 'D':
                x=4;
                y=4;
                break;
            case 'E':
                x=5;
                y=4;
                break;
            case 'F':
                x=6;
                y=4;
                break;
            case 'G':
                x=7;
                y=4;
                break;
            case 'H':
                x=8;
                y=4;
                break;
            case 'I':
                x=9;
                y=4;
                break;
            case 'J':
                x=10;
                y=4;
                break;
            case 'K':
                x=11;
                y=4;
                break;
            case 'L':
                x=12;
                y=4;
                break;
            case 'M':
                x=13;
                y=4;
                break;
            case 'N':
                x=14;
                y=4;
                break;
            case 'O':
                x=15;
                y=4;
                break;
            case 'P':
                x=0;
                y=5;
                break;
            case 'Q':
                x=1;
                y=5;
                break;
            case 'R':
                x=2;
                y=5;
                break;
            case 'S':
                x=3;
                y=5;
                break;
            case 'T':
                x=4;
                y=5;
                break;
            case 'U':
                x=5;
                y=5;
                break;
            case 'V':
                x=6;
                y=5;
                break;
            case 'W':
                x=7;
                y=5;
                break;
            case 'X':
                x=8;
                y=5;
                break;
            case 'Y':
                x=9;
                y=5;
                break;
            case 'Z':
                x=10;
                y=5;
                break;
            case ' ':
                x=9;
                y=0;
                break;
            default:
                x=0;
                y=0;
                break;
        }
        entities[i].CalcTextureCoords(dimensions, dimensions, width, height, (width*x), (height*y), LRoffset, UDoffset);
        entities[i].changeSize(changeSize);
    }
    return entities;
}

float Entity::lerp(float v0, float v1, float t) {
    return (1.0-t)*v0 + t*v1;
}

bool Entity::collisionDetect(Entity &object)    // detect a collision and change direction accordingly
{
    if (Active && object.Active)
    {
        return (top>object.bottom && bottom<object.top && left<object.right && right>object.left);
    }
    else return false;
    
};

