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
#include <vector>
#include <fstream>
#include <string>
#include <iostream>
#include <sstream>
#ifdef _WINDOWS
#define RESOURCE_FOLDER ""
#else
#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif
#include <SDL_mixer.h>

using namespace std;

// 60 FPS (1.0f/60.0f)
#define FIXED_TIMESTEP 0.05f // 10 frames per second due to my CPU
#define MAX_TIMESTEPS 6

enum gameState {titleScreen, level, gameLoss, gameWin, dying, winning};
gameState state = titleScreen;
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

int mapHeight;
int mapWidth;
int* *levelData;
int TILE_WIDTH;
int TILE_HEIGHT;

SDL_Window* displayWindow;

//=============AUDIO
int x = Mix_OpenAudio( 44100, MIX_DEFAULT_FORMAT, 2, 4096 );
Mix_Music *music = Mix_LoadMUS("music.mp3");
Mix_Chunk *loss = Mix_LoadWAV("loss.wav");
Mix_Chunk *win = Mix_LoadWAV("win.wav");
Mix_Chunk *jump = Mix_LoadWAV("jump.wav");

Entity* player;
int playerNdx;
int exitNdx;

std::vector<Entity> objects; //ALL THE ENTITIES

Matrix projectionMatrix;
Matrix modelMatrix;
Matrix viewMatrix;

// LOADING DATA FROM TILED FILE

bool readHeader(std::ifstream &stream) 
{
	string line;
	mapWidth = -1;
	mapHeight = -1;
	while(getline(stream, line)) 
	{
		if(line == "") { break; }
		istringstream sStream(line);
		string key,value;
		getline(sStream, key, '=');
		getline(sStream, value);
		if(key == "width") 
		{
			mapWidth = atoi(value.c_str());
		} 
		else if(key == "height")
		{
			mapHeight = atoi(value.c_str());
		}
        else if(key == "tilewidth")
        {
        	TILE_WIDTH = atoi(value.c_str());
        }
        else if(key == "tileheight")
        {
        	TILE_HEIGHT = atoi(value.c_str());
        }
        
	}
	if(mapWidth == -1 || mapHeight == -1) 
	{
		return false;
	} 
	else 
	{ // allocate our map data
		levelData = new int*[mapHeight];
		for(int i = 0; i < mapHeight; ++i) 
		{
			levelData[i] = new int[mapWidth];
		}
		return true;
	}
}

bool readLayerData(std::ifstream &stream) 
{
	string line;
	while(getline(stream, line)) 
	{
		if(line == "") { break; }
		istringstream sStream(line);
		string key,value;
		getline(sStream, key, '=');
		getline(sStream, value);
		if(key == "data") 
		{
			for(int y=0; y < mapHeight; y++) 
			{
				getline(stream, line);
				istringstream lineStream(line);
				string tile;
				for(int x=0; x < mapWidth; x++)
				{
					getline(lineStream, tile, ',');
					int val = atoi(tile.c_str());
					if(val > 0) 
					{
                        levelData[y][x] = val;
					} 
					else 
					{
						levelData[y][x] = 0;					
					}
				}
			}
		}
	}
	return true;
}

void placeEntity(string type, float placeX, float placeY)
{
    float height = (float) (Y_MAX - Y_MIN) / mapHeight;
    float width = 0.25f;
    float x_min=-4.55f;
    
    // CREATE PLAYER
    if(type == "Start")
    {
        objects.push_back(Entity(height, width, (float)(x_min + (placeX*width) + (width/2)), float(Y_MAX - (placeY * height) -(height / 2))));
        player = &objects[objects.size() - 1];
        playerNdx = objects.size() - 1;
        player->eType=ENTITY_PLAYER;
        player->addTexture("spritesheet_rgba.png");
        player->Static = false;
        player->CalcTextureCoords(692, 692, 21, 21, (((int)((20-1) % 30) * 23) + 2), (((int)((20-1) / 30) * 23) + 2));
        player->gravity=-0.1f;
        player->friction.x=0.50f;
    }
    
    // CREATE ENEMY
    if(type == "Enemy")
    {
        objects.push_back(Entity(width, height, (float)(x_min + (placeX*width) + (width/2)), float(Y_MAX - (placeY * height) -(height / 2))));
        Entity* temp = &objects[objects.size() - 1];
        temp->eType=ENTITY_ENEMY;
        temp->addTexture("spritesheet_rgba.png");
        temp->Static = false;
        temp->CalcTextureCoords(692, 692, 21, 21, (((int)((239-1) % 30) * 23) + 2), (((int)((239-1) / 30) * 23) + 2));
        temp->gravity=-0.1f;
        temp->friction.x=0.25f;
    }
    
    if(type == "Exit")
    {
        objects.push_back(Entity(width, height, (float)(x_min + (placeX*width) + (width/2)), float(Y_MAX - (placeY * height) -(height / 2))));
        Entity* temp = &objects[objects.size() - 1];
        exitNdx = objects.size() - 1;
        temp->eType=ENTITY_EXIT;
        temp->addTexture("spritesheet_rgba.png");
        temp->Static = false;
        temp->CalcTextureCoords(692, 692, 21, 21, (((int)((731-1) % 30) * 23) + 2), (((int)((731-1) / 30) * 23) + 2));
        temp->gravity=-0.1f;
        temp->friction.x=0.25f;
    }
    
}

bool readEntityData(std::ifstream &stream)
{
	string line;
	string type;
	while(getline(stream, line)) 
	{
		if(line == "") { break; }
		istringstream sStream(line);
		string key,value;
		getline(sStream, key, '=');
		getline(sStream, value);
		if(key == "type") 
		{
			type = value;
		} 
		else if(key == "location") 
		{
			istringstream lineStream(value);
			string xPosition, yPosition;
			getline(lineStream, xPosition, ',');
			getline(lineStream, yPosition, ',');
            float placeX = atoi(xPosition.c_str());
            float placeY = atoi(yPosition.c_str());
            placeEntity(type, placeX, placeY);
		}
	}
	return true;
}

void loadTileData(string filename)
{
	ifstream infile(filename);
	string line;
	while (getline(infile, line))
	{
		if(line == "[header]")
		{
			if(!readHeader(infile))
			{
				return;
			}
		}
		else if(line == "[layer]")
		{
            readLayerData(infile);
		}
        else if(line == "[Object Layer 1]")
        {
            readEntityData(infile);
        }
	}
}

void addLevelDataToEntity()
{
    float height = (float) (Y_MAX - Y_MIN) / mapHeight;
    float width = 0.25f;
    
    unsigned int ndx = (unsigned int)objects.size();

    for (int y=0; y<mapHeight; y++)
    {
    	for (int x=0; x<mapWidth; x++)
    	{
            if(levelData[y][x] == 0)
            {
                continue;
            }
            else
            {
                objects.push_back(Entity(width, height, (float)(-4.55f + (x*width) + (width/2)), float(Y_MAX - (y * height) -(height / 2))));
                objects[ndx].addTexture("spritesheet_rgba.png");
                objects[ndx].Static = true;
                objects[ndx].CalcTextureCoords(692, 692, 21, 21, (((int)((levelData[y][x]-1) % 30) * 23) + 2), (((int)((levelData[y][x]-1) / 30) * 23) + 2));
                objects[ndx].Static=true;
                objects[ndx].eType=ENTITY_GRASS;
                ndx++;
                
            }
    	}
    }
}


std::vector<Entity> title, titlePrompt, titleQuit, gameOver, winner;
void LoadTitles()
{
    Vector2 x;
    x.x=-2.0f;
    x.y=1.0f;
    title = Entity::printText(x, "GRADUATION", 0.5f);//TITLE
    x.y=0.0f;
    x.x=-2.0f;
    titlePrompt = Entity::printText(x, "PRESS ENTER TO START", 0.25f);//PROMPT
    x.y=-1.0f;
    x.x=-1.5f;
    titleQuit = Entity::printText(x, "PRESS Q TO QUIT", 0.25f);//PROMPT
    x.x=-2.0f;
    x.y=1.0f;
    gameOver = Entity::printText(x, "GAME OVER", 0.5f);//GAME OVER
    x.x=-1.5f;
    x.y=1.0f;
    winner = Entity::printText(x, "YOU WON", 0.5f);//WIN
}

float startPostionX, startPositionY;
void ResetPlayer()
{
    objects[playerNdx].position.x = startPostionX;
    objects[playerNdx].position.y = startPositionY;
    objects[playerNdx].CalcTextureCoords(692, 692, 21, 21, (((int)((20-1) % 30) * 23) + 2), (((int)((20-1) / 30) * 23) + 2));
    
}

void Setup() // CREATE EVERYTHING
{
    LoadTitles();
//    loadTileData(RESOURCE_FOLDER"level 1.txt");
    loadTileData(RESOURCE_FOLDER"level 2.txt");
//    loadTileData(RESOURCE_FOLDER"level 3.txt");
    addLevelDataToEntity();
    state = titleScreen;
    startPostionX = objects[playerNdx].position.x;
    startPositionY = objects[playerNdx].position.y;
    Mix_PlayMusic(music, -1);
}

float timer=6.0f;
void Update(float elapsed) // UPDATE MOVEMENT AND POSTION
{
    float matrixY=0.0f;
    float screenShakeValue=0.0f;
    switch (state)
    {
        case titleScreen:
            break;
        case dying:
            if(timer>4.0f)
            {
                Mix_PlayChannel(-1, loss, 0);
                objects[playerNdx].CalcTextureCoords(692, 692, 21, 21, (((int)((446-1) % 30) * 23) + 2), (((int)((446-1) / 30) * 23) + 2));
                timer-=elapsed;
            }
            else if(timer>2.0f)
            {
                objects[playerNdx].CalcTextureCoords(692, 692, 21, 21, (((int)((447-1) % 30) * 23) + 2), (((int)((447-1) / 30) * 23) + 2));
                timer-=elapsed;
            }
            else if(timer>0.0f)
            {
                objects[playerNdx].CalcTextureCoords(692, 692, 21, 21, (((int)((448-1) % 30) * 23) + 2), (((int)((448-1) / 30) * 23) + 2));
                timer-=elapsed;
            }
            if (timer<0.0f)
            {
                state=gameLoss;
                timer=6.0f;
            }
            screenShakeValue+=elapsed;
            viewMatrix.Translate(0.0f, sinf(screenShakeValue* 1000.0f) * 0.01f, 0.0f);
            break;
        case winning:
            if(timer>4.0f)
            {
                Mix_PlayChannel(-1, win, 0);
                objects[exitNdx].CalcTextureCoords(692, 692, 21, 21, (((int)((851-1) % 30) * 23) + 2), (((int)((851-1) / 30) * 23) + 2));
                objects[playerNdx].CalcTextureCoords(692, 692, 21, 21, (((int)((22-1) % 30) * 23) + 2), (((int)((22-1) / 30) * 23) + 2));
                timer-=elapsed;
            }
            else if(timer>2.0f)
            {
                objects[playerNdx].CalcTextureCoords(692, 692, 21, 21, (((int)((25-1) % 30) * 23) + 2), (((int)((25-1) / 30) * 23) + 2));
                timer-=elapsed;
            }
            else if(timer>0.0f)
            {
                objects[playerNdx].CalcTextureCoords(692, 692, 21, 21, (((int)((26-1) % 30) * 23) + 2), (((int)((26-1) / 30) * 23) + 2));
                timer-=elapsed;
            }
            else if (timer<0.0f)
            {
                state = gameWin;
                timer = 6.0f;
            }
            break;
        case level:
            for (int i = 0; i < objects.size(); i++) // COLLISSIONS
            {
                // CHECK COLLISION
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
                                if (fabs(objects[i].position.y - objects[j].position.y) <0.1f)
                                {
                                    continue;
                                }
                                else if(objects[i].velocity.y<0)
                                {
                                    objects[i].position.y+=(objects[i].getYPenetrationDown(objects[j]))+0.01f;
                                    objects[i].CollidedBottom=true;
                                    if(objects[i].eType==ENTITY_PLAYER)
                                    {
                                        Jumping=false;
                                    }
                                }
                                else
                                {
                                    objects[i].position.y+=(objects[i].getYPenetrationUp(objects[j]))-0.01f;
                                    objects[i].CollidedTop=true;
                                    objects[i].acceleration.y=0.0f;
                                }
                                objects[i].velocity.y=0.0f;
                            }
                            else if (objects[j].eType==ENTITY_ENEMY && objects[i].eType==ENTITY_PLAYER)
                            {
                                state=dying;
                            }
                            else if (objects[j].eType==ENTITY_EXIT && objects[i].eType==ENTITY_PLAYER)
                            {
                                state=winning;
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
                                if (fabs(objects[i].position.x - objects[j].position.x) <0.1f)
                                {
                                    continue;
                                }
                                else if(objects[i].velocity.x>0)
                                {
                                    objects[i].position.x-=(objects[i].getXPenetrationRight(objects[j]));
                                    //                            objects[i].position.x+=0.01f;
                                    objects[i].CollidedRight=true;
                                }
                                else
                                {
                                    objects[i].position.x+=fabsf(objects[i].getXPenetrationLeft(objects[j]));
                                    //                            objects[i].position.x+=0.01f;
                                    objects[i].CollidedLeft=true;
                                }
                                objects[i].velocity.x=0.0f;
                            }
                            else if (objects[j].eType==ENTITY_ENEMY && objects[i].eType==ENTITY_PLAYER)
                            {
                                state=dying;
                            }
                            else if (objects[j].eType==ENTITY_EXIT && objects[i].eType==ENTITY_PLAYER)
                            {
                                state=winning;
                            }
                        }
                    }
                }
                if(objects[i].eType==ENTITY_ENEMY)
                {
                    float distance = objects[i].position.x - objects[playerNdx].position.x;
                    if(distance>2.0f)
                    {
                        //                objects[i].velocity.x=0.0f;
                        objects[i].acceleration.x=0.0f;
                    }
                    else if(distance<-2.0f)
                    {
                        //                objects[i].velocity.x=0.0f;
                        objects[i].acceleration.x=0.0f;
                    }
                    else if(distance>-2.0f && distance<0.0f)
                    {
                        //                objects[i].velocity.x=0.5f;
                        objects[i].acceleration.x+=0.1f;
                    }
                    else if(distance<2.0f && distance>0.0f)
                    {
                        //                objects[i].velocity.x=-0.5f;
                        objects[i].acceleration.x-=0.1f;
                    }
                    
                }
            }
            if(objects[playerNdx].position.x>-3.9f && objects[playerNdx].position.x<4.3f)
            {
                if(Jumping) {matrixY=-0.1f;}
                else {matrixY =0.0f;}
                viewMatrix.setPosition(-objects[playerNdx].position.x, matrixY, 0.0f);
            }
            if(objects[playerNdx].position.y<-1.8f)
            {
                state=dying;
            }
            break;
        case gameLoss:
            if (timer>0.0f)
            {
                timer-=elapsed;
            }
            else
            {
                state = titleScreen;
                timer = 6.0f;
            }
            break;
        case gameWin:
            if (timer>0.0f)
            {
                timer-=elapsed;
            }
            else
            {
                state = titleScreen;
                timer = 6.0f;
            }
            break;
    }
}

void Render(ShaderProgram &program)
{
    switch (state)
    {
        case titleScreen:
            viewMatrix.identity();
            for (int i=0; i<title.size(); i++) {
                title[i].Draw(program);     //TOP LINE OF TEXT
            }
            for (int i=0; i<titlePrompt.size(); i++) {
                titlePrompt[i].Draw(program);    //BOTTOM PROMPT
            }
            for (int i=0; i<titleQuit.size(); i++) {
                titleQuit[i].Draw(program);    //BOTTOM PROMPT
            }
            program.setLightPosition(0.0f, 0.0f);
            break;
        
        case dying:
        case level:
            for (int i=(int)objects.size()-1; i>-1; i--) //SO THAT THE PLAYER IS RENDERED IN FRONT
            {
                objects[i].Draw(program);
            }
            program.setLightPosition(startPostionX, startPositionY);
            break;
        case winning:
            for (int i=(int)objects.size()-1; i>-1; i--) //SO THAT THE PLAYER IS RENDERED IN FRONT
            {
                objects[i].Draw(program);
            }
            program.setLightPosition(objects[exitNdx].position.x, objects[exitNdx].position.y);
            break;
        case gameLoss:
            viewMatrix.identity();
            for (int i=0; i<gameOver.size(); i++) {
                gameOver[i].Draw(program);    //BOTTOM PROMPT
            }
            program.setLightPosition(0.0f, 0.0f);
            break;
        case gameWin:
            viewMatrix.identity();
            for (int i=0; i<winner.size(); i++) {
                winner[i].Draw(program);    //BOTTOM PROMPT
            }
            program.setLightPosition(0.0f, 0.0f);
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
    
    projectionMatrix.setOrthoProjection(X_MIN, X_MAX, Y_MIN, Y_MAX, Z_MIN, Z_MAX);
    
    glUseProgram(program.programID);
    
    Setup();
    
    float lastFrameTicks = 0.0f;
    SDL_Event event;
    bool done = false;
    
    const Uint8 *keys = SDL_GetKeyboardState(NULL);
    
    while (!done) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE)
            {
                done = true;
            }
        }
        switch (state)
        {
            case titleScreen:
                if(keys[SDL_SCANCODE_RETURN])
                {
                    ResetPlayer();
                    state=level;
                }
                if(keys[SDL_SCANCODE_Q])
                {
                    done = true;
                }
                break;
            case level:
                if(keys[SDL_SCANCODE_RIGHT])
                {
                    objects[playerNdx].acceleration.x=0.5f;
                }
                if(keys[SDL_SCANCODE_LEFT])
                {
                    objects[playerNdx].acceleration.x=-0.5f;
                }
                if(keys[SDL_SCANCODE_SPACE])
                {
                    if(!Jumping)
                    {
                        objects[playerNdx].velocity.y=2.0f;
                        Mix_PlayChannel(-1, jump, 0);
                        Jumping=true;
                    }
                }
            case dying:
            case winning:
                if(keys[SDL_SCANCODE_ESCAPE])
                {
                    state = titleScreen;
                }
            case gameLoss:
            case gameWin:
                break;
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
        switch (state)
        {
            case titleScreen:
                glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
                break;
            case dying:
                glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
                break;
            case winning:
                glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
                break;
            case level:
                glClearColor(0.7f, 0.7f, 0.7f, 1.0f);
                break;
            case gameLoss:
                glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
                break;
            case gameWin:
                glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
                break;
        }

        
        program.setProjectionMatrix(projectionMatrix);
        program.setViewMatrix(viewMatrix);
//        program.setLightPosition(0.0f, 0.0f);
        Render(program);
        
        SDL_GL_SwapWindow(displayWindow);
    }
    
    
    
//    Mix_FreeChunk(laser);
//    Mix_FreeChunk(explosion);
//    Mix_FreeMusic(music);
    SDL_Quit();
    return 0;
}
