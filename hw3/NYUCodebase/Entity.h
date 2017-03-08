#include "Matrix.h"
#include <SDL_image.h>
#include "ShaderProgram.h"
#include "stb_image.h"
#include <vector>

struct Vector2
{
    float x;
    float y;
};

class Entity
{
public:
	Vector2 position;
	Vector2 direction;
	Vector2 length;
    Vector2 acceleration;
    Vector2 friction;
    
    float gravity;
	float top;
	float bottom;
	float right;
	float left;
    
	float textureTop;
	float textureBottom;
	float textureRight;
	float textureLeft;
    
    float textureCoords[12];
    GLuint texture;
    static GLuint LoadTexture(const char *filePath);
    void addTexture(std::string name);
    
    bool Debug;
    bool Active;
    
    Matrix modelMatrix;

    Entity(float Width=0.0f, float Height=0.0f, float x=0.0f, float y=0.0f);
    void updateAll();
    void updateLength(Vector2 newLength);
    
    void Draw(ShaderProgram &program);
    void Move(float ticks);
    void CalcTextureCoords(int totalWidth, int totalHeight, int Width, int Height, int Left, int Top, int LRoffset=0, int UDoffset=0);
    void changeSize(float changeByThisMuch);

    static std::vector<Entity> printText(Vector2 postion, std::string line, float changeSize);
    float lerp(float v0, float v1, float t);
    bool collisionDetect(Entity &object);
};
