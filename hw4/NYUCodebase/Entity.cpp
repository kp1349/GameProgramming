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
#include <math.h>

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
    rotationMatrix = Matrix();
    scaleMatrix = Matrix();
    translationMatrix = Matrix();
    
    length.x=(float)Width/2;
    length.y=(float)Height/2;
    position.x=x;
    position.y=y;
    
    textureTop=0.0f;
    textureBottom=1.0f;
    textureLeft=0.0f;
    textureRight=1.0f;
    
    velocity.x=0.0f;
    velocity.y=0.0f;
    acceleration.x=0.0f;
    acceleration.y=0.0f;
    gravity=0.0f;
    friction.x=0.0f;
    friction.y=0.0f;
    ratio=1.0f;
    
    Debug=true;
    Active=true;
    Static=true;
    
    rotationAngle=0.0f;
    scale.x=1.0f;
    scale.y=1.0f;
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
    if(Active)
    {
        program.setModelMatrix(modelMatrix);
        glBindTexture(GL_TEXTURE_2D, texture);
        float vertices[] =
        {
            length.x, length.y,
            (-1.0f*length.x), length.y,
            (-1.0f*length.x), (-1.0f*length.y),
            // length.x, length.y,
            // (-1.0f*length.x), (-1.0f*length.y),
            length.x, (-1.0f*length.y)
        };
        
        // float textureVertices[] =
        // {
        //     textureRight, textureTop,
        //     textureLeft, textureTop,
        //     textureLeft, textureBottom,
        //     textureRight, textureTop,
        //     textureLeft, textureBottom,
        //     textureRight, textureBottom
        // };
        
        translationMatrix.setPosition(position.x, position.y, 0.0f);
        rotationMatrix.setRoll(rotationAngle);
        scaleMatrix.setScale(scale.x, scale.y, 1.0f);
        
        modelMatrix.identity();
        modelMatrix = modelMatrix * scaleMatrix;
        modelMatrix = modelMatrix * rotationMatrix;
        modelMatrix = modelMatrix * translationMatrix;
        
        glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
        glEnableVertexAttribArray(program.positionAttribute);
        // glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, textureVertices);
        // glEnableVertexAttribArray(program.texCoordAttribute);
        glDrawArrays(GL_LINE_LOOP, 0, 4);
        glDisableVertexAttribArray(program.positionAttribute);
        // glDisableVertexAttribArray(program.texCoordAttribute);
    }
}

void Entity::CalcTextureCoords(int totalWidth, int totalHeight, int Width, int Height, int Left, int Top, int LRoffset, int UDoffset)
{
    textureTop=(float)(Top+UDoffset)/totalHeight;
    textureBottom=(float)(Top+Height-UDoffset)/totalHeight;
    textureLeft=(float)(Left+LRoffset)/totalWidth;
    textureRight=(float)(Left+Width-LRoffset)/totalWidth;
    ratio=(float)Width/Height;
    length.x*=ratio;
}

void Entity::changeSize(float changeByThisMuch)
{
    length.x*=changeByThisMuch;
    length.y*=changeByThisMuch;
    // updateAll();
}

void Entity::Move(float ticks)
{
    velocity.x = lerp(velocity.x, 0.0f, ticks*friction.x);
    velocity.y = lerp(velocity.y, 0.0f, ticks*friction.y);
    velocity.x += acceleration.x*ticks;
    velocity.y += acceleration.y*ticks;
    acceleration.x=0;
    acceleration.x=0;
    float moveRight=velocity.x*ticks;
    float moveUp=velocity.y*ticks;
    position.y+=moveUp;
    position.x+=moveRight;
}

void Entity::MoveX(float ticks)
{
    velocity.x = lerp(velocity.x, 0.0f, ticks*friction.x);
    velocity.x += acceleration.x*ticks;
    acceleration.x=0.0f;
    position.x+=(velocity.x*ticks);
}

void Entity::MoveY(float ticks)
{
    velocity.y += acceleration.y*ticks + gravity;
    velocity.y = lerp(velocity.y, 0.0f, ticks*friction.y);
    acceleration.y=0.0f;
    position.y+=(velocity.y*ticks);
}

float Entity::getYPenetration(Entity object)
{
    float penetration = fabs((position.y - object.position.y) - length.y - object.length.y);
    return penetration;
}

float Entity::getXPenetration(Entity object)
{
    float penetration = fabs((position.x - object.position.x) - length.x - object.length.x);
    return penetration;
}


std::vector<Entity> Entity::printText(Vector Position, std::string line, float changeSize)
{
    std::vector<Entity> entities;
    int LRoffset=8;
    int UDoffset=5;
    int dimensions=512;
    int width=32;
    int height=32;
    float increment = (changeSize*1.0f);
    increment-=increment*((float)LRoffset/width);
    float current = Position.x;
    for (int i=0; i<line.length(); i++) {
        entities.push_back(Entity(1.0f, 1.0f, current, Position.y));
        current+=increment;
        entities[i].addTexture("font1.png");
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

float Entity::lerp(float v0, float v1, float t)
{
    return (1.0-t)*v0 + t*v1;
}

bool Entity::collisionDetect(Entity object)    // detect a collision and change velocity accordingly
{
    if (Active && object.Active)
    {
        return ((position.y+length.y)>(object.position.y-object.length.y) && (position.y-length.y)<(object.position.y+object.length.y) && (position.x-length.x)<(object.position.x+object.length.x) && (position.x+length.x)>(object.position.x-object.length.x));
    }
    else return false;
}


void Entity::ResetCollisions()
{
    CollidedTop=false;
    CollidedBottom=false;
    CollidedLeft=false;
    CollidedRight=false;
};



// Vector methods

Vector::Vector()
{
    x=0.0f;
    y=0.0f;
    z=0.0f;
}
Vector::Vector(float X, float Y, float Z)
{
    x=X;
    y=Y;
    z=Z;
}
float Vector::length() const
{
    return sqrt((x*x) + (y*y));
}

void Vector::normalize()
{
    float len=length();
    if (len==0)
    {
        x=0.0f;
        y=0.0f;
        z=0.0f;
    }
    else
    {
        x/=len;
        y/=len;
        z/=len;
    }
}

//Vector Vector::operator * (const Vector &v)
//{
//    return Vector((x * v.x), (y * v.y), (z * v.z));
//}

void Entity::setVertices()
{
    translationMatrix.setPosition(position.x, position.y, 0.0f);
    rotationMatrix.setRoll(rotationAngle);
    scaleMatrix.setScale(scale.x, scale.y, 1.0f);
    
    modelMatrix.identity();
    modelMatrix = modelMatrix * scaleMatrix;
    modelMatrix = modelMatrix * rotationMatrix;
    modelMatrix = modelMatrix * translationMatrix;

    Vector topRight = Vector(length.x, length.y, 0.0f);
    Vector topLeft = Vector((-1.0f*length.x), length.y, 0.0f);
    Vector bottomRight = Vector(length.x, (-1.0f*length.y), 0.0f);
    Vector bottomLeft = Vector((-1.0f*length.x), (-1.0f*length.y), 0.0f);
    
    topRight = topRight * modelMatrix;
    topLeft = topLeft * modelMatrix;
    bottomRight = bottomRight * modelMatrix;
    bottomLeft = bottomLeft * modelMatrix;

    vertices.clear();
    
    vertices.push_back(topRight);
    vertices.push_back(topLeft);
    vertices.push_back(bottomRight);
    vertices.push_back(bottomLeft);
}

Vector Vector::operator * (const Matrix &mat) const
{
    Vector temp = Vector();
    float vec [4] = {x, y, z, 1.0f};
    temp.x = (mat.m[0][0] * vec[0]) + (mat.m[1][0] * vec[1]) + (mat.m[2][0] * vec[2]) + (mat.m[3][0] * vec[3]);
    temp.y = (mat.m[0][1] * vec[0]) + (mat.m[1][1] * vec[1]) + (mat.m[2][1] * vec[2]) + (mat.m[3][1] * vec[3]);
    temp.z = (mat.m[0][2] * vec[0]) + (mat.m[1][2] * vec[1]) + (mat.m[2][2] * vec[2]) + (mat.m[3][2] * vec[3]);
    return temp;
}
