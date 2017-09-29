#pragma once

// #include <sp>
//#include <spritebatch.h>
//
//#include "glad.c"
//#include "gl/IOManager.cpp"
//#include "gl/GLSLProgram.cpp"
//#include "gl/picoPNG.cpp" 
// #include "gl/ImageLoader.cpp"

#include  <GLTexture.h> 
#include <TextureCache.h>
#include <SpriteBatch.h>
#include <SpriteFont.h>
#include <TextureCache.h>
#include <Camera2D.h>
// #include "gl/TileSheet.h"
// #include <>


// #include <Sprite.h>

//#include "gl/ResourceManager.cpp"
//#include "gl\SpriteBatch.cpp"
//

// #include <ImageLoader.h>
// #include <gl/GL.h>


// #include "gl/Camera2D.cpp"

// #include "gl\GLSLProgram.cpp"
// #include "gl/SpriteFont.cpp"
// #include "gl/DebugRenderer.cpp"

// todo: korjaa oikea resurrsi managers

#define GetGameState(core) game_state*   gameState = (game_state*)(core)->memory->permanentStorage;
#define DefineInput(core)  InputManager* input     = (core)->input;

struct ResourceManager
{
	GLuint(*SurfaceToGlTexture)(SDL_Surface*);
	void(*FreeTexture)(GLuint*);
	SDL_Surface*(*LoadSurface)(const char*);
};


// shitty resource manager 
struct Context
{
	UpiEngine::TextureCache* textureCacheCtx;
};

// tänne tavallaan public engine hommat
struct EngineCore
{
	UpiEngine::DebugRenderer* debugger; // pitäsiköhän nämä jakaa omaan init/context, joka asettaisi nämä oikein
										// vai tekisiköhän accesille omat macror
										// TODO: testaa ylempiä käytännössä
	InputManager*  input;
	game_memory*   memory;
	// UpiEngine::TileSheet*  	   testyTexture;
	UpiEngine::Camera2D*   	   camera2D;
	GLuint 			slopeMapTexture; // todo: remove

	SDL_GLContext *glcontext;
	SDL_Window    *window;
	UpiEngine::SpriteBatch *spriteBatch;
	
	Context       ctx;

	scripting     script;          // lua scipts
	FileWatcher   filewatcher;     // very bare-bones file write time poller

	ResourceManager resources;

	void(*AddToConsole)(const char* text);

	float deltaTime;
	float timeMultiplier;

	int       screenWidth;
	int       screenHeight;

	// boring
	bool      pause;
	bool      advanceNextFrame;
	bool      advancedLastFrame;
	bool      beginSkipToFrame;
	float     timeFactor;
	int       skipToFrame;
};

