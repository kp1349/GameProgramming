#include "Matrix.h"
#include <SDL_image.h>
#include "ShaderProgram.h"
#include "stb_image.h"
#include <vector>

enum EntityType {ENTITY_PLAYER, ENTITY_ENEMY, ENTITY_GRASS, ENTITY_EXIT};

struct Vector2
{
    float x;
    float y;
};

class Entity
{
public:
	Vector2 position;
	Vector2 velocity;
	Vector2 length;
    Vector2 acceleration;
    Vector2 friction;
    
    float gravity;
	float top;
	float bottom;
	float right;
	float left;
    float ratio;
    
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
    bool Static;
    
    Matrix modelMatrix;

    Entity(float Width=0.5f, float Height=0.5f, float x=0.0f, float y=0.0f);
    void updateAll();
    void updateLength(Vector2 newLength);
    
    void Draw(ShaderProgram &program);
    void Move(float ticks);
    void CalcTextureCoords(int totalWidth, int totalHeight, int Width, int Height, int Left, int Top, int LRoffset=0, int UDoffset=0);
    void changeSize(float changeByThisMuch);

    static std::vector<Entity> printText(Vector2 postion, std::string line, float changeSize);
    float lerp(float v0, float v1, float t);
    bool collisionDetect(Entity object);

    void MoveX(float ticks);
    void MoveY(float ticks);
    float getYPenetrationDown(Entity object);
    float getXPenetrationLeft(Entity object);
    float getYPenetrationUp(Entity object);
    float getXPenetrationRight(Entity object);
    
    bool CollidedTop;
    bool CollidedBottom;
    bool CollidedRight;
    bool CollidedLeft;
    void ResetCollisions();
    EntityType eType;
};
