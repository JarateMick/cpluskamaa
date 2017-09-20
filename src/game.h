#pragma once
#define EXPORT extern "C" 
// __declspec( dllexport )

#define kiloBytes(value) ((value)*1024LL)
#define megaBytes(value) (kiloBytes(value)*1024LL)
#define gigaBytes(value) (megaBytes(value)*1024LL)
#define teraBytes(value) (gigaBytes(value)*1024LL)
#define ArrayCount(array) ( sizeof(array) / sizeof((array)[0]))

#include "Debug.h"
#include "Random.cpp"

#define ASSERTIONS_ENABLED 1
#if ASSERTIONS_ENABLED
#define debugBreak() __debugbreak();

#define ASSERT(expr)   \
	if (expr) { } \
	else \
	{    \
		Debug::log("%s, %s, %d\n", #expr, __FILE__, __LINE__); \
		debugBreak();                 \
	}
#else
#define ASSERT(expr) 
#endif

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN 1
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 1
#endif

#define GetGameState(core) game_state*   gameState = (game_state*)(core)->memory->permanentStorage;
#define DefineInput(core)  InputManager* input     = (core)->input;

#include <math.h>
#define CLAMP(x, upper, lower) (std::min(upper, std::max(x, lower)))


// #define 

struct v2 { int x, y; };


#include <lua.hpp>

#include <Windows.h>
 // #define SOL_EXCEPTIONS_SAFE_PROPAGATION
//#include <sol\sol.hpp>


#include <cstdint>
#include <vector>
#include "TextureHolder.h"
#include <functional>
#include <SDL2\SDL_image.h>


// #include "imgui/imgui.h"

// #include "TextureHolder.h"
// #include "SDL_FontCache/SDL_FontCache.c"

#define Internal static

#include "Hex.h"

// lajitte engine kamat omaan ja game kamat omaan

// engineCOre jne..
// entity

// gamestate jne...


// TODO: Vec2  -> typedeffaa vec jne....
// oma vs Glmstruct
// oma: 
//  + nopeampi compileta
//  + vecit helppoja tehdä


// glm: 
//   + mat4 voi olla vaikea tehdä itse
Internal FILETIME Win32GetLastWriteTime(const char* path)
{
	FILETIME time = {};
	WIN32_FILE_ATTRIBUTE_DATA data;

	if (GetFileAttributesEx(path, GetFileExInfoStandard, &data))
		time = data.ftLastWriteTime;

	return time;
}

enum ResourceType
{
	Resource_shader = 0,
	Resource_script,
	Resource_texture,
	Resource_max,
};

struct resourceData 
{
	std::vector<FILETIME> filetimes;
	std::vector<std::string> filepathsToWatch;
	int watchFilesCount;
};

Uint8 getAlpha(Uint32 color, SDL_PixelFormat* fmt)
{
	Uint32 temp;
	Uint32 pixel = color;
	temp = pixel & fmt->Amask;  /* Isolate alpha component */
	temp = temp >> fmt->Ashift; /* Shift it down to 8-bit */
	temp = temp << fmt->Aloss;  /* Expand to a full 8-bit number */

	return (Uint8)temp;
}

class ImageData
{
public:
	SDL_Surface* surface = 0;

	ImageData(const char* filename);

	~ImageData()
	{
		SDL_FreeSurface(surface);
	}

	void   set_pixel(int x, int y, Uint32 color)
	{
		// Uint32 *target_pixel = (Uint32 *)surface->pixels + y * surface->pitch +
			// x * sizeof *target_pixel;
		// *target_pixel = pixel;

		Uint8 * pixel = (Uint8*)surface->pixels;
		pixel += (y * surface->pitch) + (x * sizeof(Uint32));
		*((Uint32*)pixel) = color;
	}

	Uint32 GetPixel(int x, int y)
	{
		// return (Uint32)surface->pixels + y * surface->pitch +
		//	x * sizeof(Uint32);

		return ((unsigned int*)surface->pixels)[y*(surface->pitch / sizeof(unsigned int)) + x];
	}
}; 


struct FileWatcher
{
	// std::vector<FILETIME> filetimes;
	// std::vector<std::string> filepathsToWatch;
	// int watchFilesCount;

	resourceData resources[Resource_max];

	// load from config file !!!
	void init(const char** filenames, int size, ResourceType type)
	{
		resourceData& res = resources[type];

		res.watchFilesCount = size;

		for (int i = 0; i < res.watchFilesCount; i++)
		{
			res.filepathsToWatch.push_back(std::string(filenames[i]));
		}

		for (int i = 0; i < res.watchFilesCount; i++)
		{
			res.filetimes.push_back(Win32GetLastWriteTime(res.filepathsToWatch[i].c_str()));
		}
	}



	// ladataanko vai ei
	const char* update(ResourceType type)
	{
		resourceData& res = resources[type];

		for (int i = 0; i < res.watchFilesCount; i++)
		{
			FILETIME newFileTime = Win32GetLastWriteTime(res.filepathsToWatch[i].c_str());
			if (CompareFileTime(&newFileTime, &res.filetimes[i]))
			{
				// reload this file!
				res.filetimes[i] = newFileTime;
				return res.filepathsToWatch[i].c_str();
			}
		}
		return 0;
	}

	void showImgui(ResourceType type)
	{
		
	}


};

struct game_memory
{
	bool isInitialized;

	uint64_t permanentStorageSize;
	void* permanentStorage;

	uint64_t transientStorageSize;
	void* transientStorage;
};



struct scripting
{
	// void(*executeCommand)(const char* script);
	// sol::state* luaP;
	lua_State* L;

	const char* executeCommand(const char* script)
	{
		static char buffer[1024];
		int succ = luaL_loadstring(L, script);
		if (succ != 0)
		{
			// fprintf(stderr, "%s\n", lua_tostring(L, -1));
			sprintf(buffer, "[error]: %s\n", lua_tostring(L, -1));
			printf("%s", buffer);
			return buffer;
		}

		int ret = lua_pcall(L, 0, 0, 0);
		if (ret != 0)
		{
			// fprintf(stderr, "%s\n", lua_tostring(L, -1));
			sprintf(buffer, "[error]: %s\n", lua_tostring(L, -1));
			printf("%s", buffer);
			return buffer;
			// debugBreak();
		}
		lua_settop(L, 0);

		return 0;
	}
};






#include "glad.c"
#include "gl/IOManager.cpp"
#include "gl/GLSLProgram.cpp"
#include "gl/picoPNG.cpp" 
#include "gl/ImageLoader.cpp"
#include "gl/GLTexture.h"
#include "gl/TextureCache.cpp"
#include "gl/ResourceManager.cpp"
#include "gl\SpriteBatch.cpp"


#include "gl/Camera2D.cpp"
#include "gl/TileSheet.h"

// #include "gl\GLSLProgram.cpp"
#include "gl/SpriteFont.cpp"
#include "gl/DebugRenderer.cpp"

// todo: korjaa oikea resurrsi managers
struct ResourceManager
{
	GLuint(*SurfaceToGlTexture)(SDL_Surface*);
	void(*FreeTexture)(GLuint*);
	SDL_Surface*(*LoadSurface)(const char*);
};



// tänne tavallaan public engine hommat
struct EngineCore
{
	UpiEngine::DebugRenderer* debugger; // pitäsiköhän nämä jakaa omaan init/context, joka asettaisi nämä oikein
										// vai tekisiköhän accesille omat macror
										// TODO: testaa ylempiä käytännössä
	InputManager*  input;
	game_memory*   memory;
	UpiEngine::TileSheet*  	   testyTexture;
	UpiEngine::Camera2D*   	   camera2D;
	GLuint 			slopeMapTexture; // todo: remove

	SDL_GLContext *glcontext;
	SDL_Window    *window;

	UpiEngine::SpriteBatch *spriteBatch;

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


class StackAllocator
{
public:
	typedef uint32_t Marker;

	void init(uint32_t stackSize_bytes, void* memoryStart)
	{
		memorySize = stackSize_bytes;
		memory = memoryStart;
		bottom = (uint32_t)memoryStart;
		top = 0;
	}

	void* alloc(uint32_t size_bytes)
	{
		void* result = (void*)((uint32_t)memory + top);
		top += size_bytes;
		return result;
	}

	void freeToMarker(Marker marker)
	{
		top = marker;
	}

	void clear()
	{
		top = bottom;
	}
	// Marker getMarker();

	Marker top;
	Marker bottom;
	void*  memory;
	Marker memorySize;
};


typedef size_t memory_index;
struct memory_arena
{
	memory_index    size;
	uint8_t        *base;
	memory_index    used;

	void InitalizeArena(memory_index size, uint8_t *base)
	{
		this->size = size;
		this->base = base;
		this->used = 0;
	}
};

#define PushStruct(Arena, type) (type *)PushSize_(Arena, sizeof(type))
#define PushArray(Arena, Count, type) (type *)PushSize_(Arena, (Count)*sizeof(type))

void* PushSize_(memory_arena *Arena, memory_index Size)
{
	ASSERT((Arena->used + Size) <= Arena->size);
	void *result = Arena->base + Arena->used;
	Arena->used += Size;

	return result;
}

// struct TilemapEditor
// {
	// bool EditMode;
	// int  SelectionRadius;
// };

#include "Entity.h"
// #include "Tilemap.h"

void FloodFillImage(ImageData* imageData, ImageData* replacement, int startX, int startY, Uint32 targetColor, Uint32 replacementColor)
{
	if (targetColor == imageData->GetPixel(startX, startY) && replacement->GetPixel(startX, startY) != replacementColor)
	{
		replacement->set_pixel(startX, startY, replacementColor);
		FloodFillImage(imageData, replacement, startX + 1, startY, targetColor, replacementColor);
		FloodFillImage(imageData, replacement, startX - 1, startY, targetColor, replacementColor);
		FloodFillImage(imageData, replacement, startX, startY + 1, targetColor, replacementColor);
		FloodFillImage(imageData, replacement, startX, startY - 1, targetColor, replacementColor);
	}
	else
	{
		return;
	}
}

struct WorldMapEditor
{
	Uint32 editorColor;
	int inputProvinceId;
	float inputX;
	float inputY;
};

struct WorldMap
{
	GLuint temptextureid;
	ImageData provinces;
	ImageData visual;

	// Uint32 editorColor;
	WorldMapEditor editor;

	glm::vec4 dimensions;

	void Draw(UpiEngine::SpriteBatch* sb)
	{
		sb->draw(dimensions, glm::vec4{ 0.f, 0.f, 1.f, 1.f }, temptextureid, 1.0f);

		// minimap :)
		// sb->draw(glm::vec4{ 0.f, 0.f, 590.f, 480.f }, glm::vec4{ 0.f, 0.f, 1.f, 1.f }, temptextureid, 1.f);
	}

	Uint32 GetPixelSideFromWorld(int x, int y)
	{
		return provinces.GetPixel(x / 4, 480.f - y / 4); // real size 
	}

	Uint32 GetCurrentHolder(int x, int y)
	{
		return visual.GetPixel(x / 4, 480.f - y / 4);
	}

	Uint32 GetPixelSide(int x, int y)
	{
		// contains ?
		return provinces.GetPixel(x, y); // real size 
	}

	void changeSideWorld(int x, int y, Uint32 to, EngineCore* core)
	{
		x = (x - dimensions.x) / 4;
		y = (dimensions.w - y) / 4;
		Uint32 targetColor = GetPixelSide(x, y);

		SDL_PixelFormat *fmt = provinces.surface->format;
		Uint8 alpha = getAlpha(targetColor, fmt);
		if (targetColor != 0xFF000000 && alpha != 0)
		{
			FloodFillImage(&provinces, &visual, x, y, targetColor, to);

			// taman jos tekee framen lopussa niin ei tarvitse tehda kuin yksi tekstruuri generaatio
			core->resources.FreeTexture(&temptextureid);
			temptextureid = core->resources.SurfaceToGlTexture(visual.surface);
		}
	}

	void CheckSide(Uint32 targetColor)
	{

	}
};


struct ProvinceData
{
	int                    maxProvinces;
	Uint32*                idToColor;
	v2*                    positions;
	std::map<Uint32, int>* colorToId;
	int*                   currentCount;
};

struct game_state
{
	memory_arena   arena;
	Entity         entities[10000];
	Entity*        player;
	int            currentEntityCount;
	ProvinceData   provinceData;

	// TileMap tilemap;
	// TilemapEditor editor;
	WorldMap worldmap;
	

	float cameraSpeed;
};

// jEntity* GetFirstAvaibleEntity(game_state* state);
Entity* GetFirstAvaibleEntity(game_state* state)
{
	ASSERT(state->currentEntityCount < ArrayCount(state->entities));
	return &state->entities[state->currentEntityCount++];
}

Entity* newEntity(float x, float y, Entity_Enum type, game_state* state)
{
	Entity* result = GetFirstAvaibleEntity(state);
	result->x = x;
	result->y = y;
	result->type = type;
	return result;
}

bool buildBuilding(float x, float  y, building_type type, game_state* state, Uint32 side)
{
	WorldMap* map = &state->worldmap;
	
	// pelaajaan tuo check
	// map->GetPixelSideFromWorld((float)x, (float)y);

	// vaativa check voiko rakennuksen rakentaa tahan // rakennus bitmap? // probably tarkistetaan vain ymparisto
													  // quad tree alkoi kuulostaa kivalta
	Entity* e = newEntity(x, y, Entity_building, state);
	e->building.type  = type;
	e->building.side  = side;
	e->building.timer = 0.f;  

	return true;
}

void f(Entity *e, EngineCore* core)
{
	if (auto ninja = GET_ENTITY(e, ninja))
	{
		int x{ 0 };
		int y{ 0 };
		if (core->input->isKeyDown(SDL_SCANCODE_UP))
			--y;
		if (core->input->isKeyDown(SDL_SCANCODE_DOWN))
			++y;
		if (core->input->isKeyDown(SDL_SCANCODE_LEFT))
			--x;
		if (core->input->isKeyDown(SDL_SCANCODE_RIGHT))
			++x;

		if (x != 0 || y != 0)
		{
			// glm::vec2 normal = glm::normalize(glm::vec2{x, y});
			// e->pos.x += (normal.x * core->deltaTime * 0.5f);
			// e->pos.y += (normal.y * core->deltaTime * 0.5f);
			e->x += (float)x * core->deltaTime * 25.f;
			e->y += (float)y * core->deltaTime * 25.f;
			//printf("%f", y * core->deltaTime * 0.5f);
		}
		//e->y += sinf(e->x) * 5;
	}
	else if (auto npc = GET_ENTITY(e, npc))
	{
		if (core->input->isKeyPressed(SDL_SCANCODE_SPACE))
		{
			// printf("ninjas are gays!\n");
		}
	}
	else if (auto entity = GET_ENTITY(e, script))
	{
		GetGameState(core);
		DefineInput(core);

		// nuista clikattavista voisi laittaa jonnekin mukavammin saataville core->clicables
		// tai sitten tekisi entity updatesta systeemi maisen

		if (core->input->isMouseClicked(1))
		{
			for (int i = 0; i < gameState->currentEntityCount; i++) {

				Entity& e = gameState->entities[i];
				if (e.type == Entity_script)
				{
					auto script = GET_ENTITY(&e, script);

					if (script->hitbox.Contains(input->mouse.x, input->mouse.y))
					{
						// hitbox clicked! show message
						printf("hitbox clicked!\n");
					}
				}
			}
		}
	}
	else if (auto player = GET_ENTITY(e, player))
	{
		GetGameState(core);
		DefineInput(core);

		// lista controlloiduista entitytyista id / pointerit
		// tehdaan valintoja sen perusteella rectilla -> vetaa nelion
		// sitten voi kotrolloida uniitteja 

		// jos clickkaa / painaa pikanappainta voi rakentaa rakennuksia

		if (input->isMouseClicked(1)) // tutki input juttua enemman
		{
			printf("click\n");

			// valitse -> menusta rakennettavaksi rakennus!
			if (player->selectedBuildingType != building_none)
			{
				// build that building
				buildBuilding(input->mouse.x, input->mouse.y, player->selectedBuildingType, gameState, player->side);

				player->selectedBuildingType = building_none;
			}

			// command dem selected troops / select some troops
			// 1 -> select
			// 2 -> clear selection
		}
	}
	else if (auto entity = GET_ENTITY(e, unit))
	{
		GetGameState(core);
		DefineInput(core);

		// if (entity->side == gameState.playerSide)
		// {
		// }
		Uint32 side = gameState->worldmap.GetCurrentHolder((int)e->x, (int)e->y);
		if (side != entity->side)
		{
			gameState->worldmap.changeSideWorld((int)e->x, (int)e->y, entity->side, core);
		}

		glm::vec2 playerMoveVev{ 0.f, 0.f };
		if (input->isKeyDown(SDL_SCANCODE_DOWN))
		{
			playerMoveVev.y -= 1.f;
		}
		if (input->isKeyDown(SDL_SCANCODE_UP))
		{
			playerMoveVev.y += 1.f;
		}
		if (input->isKeyDown(SDL_SCANCODE_LEFT))
		{
			playerMoveVev.x -= 1.f;
		}
		if (input->isKeyDown(SDL_SCANCODE_RIGHT))
		{
			playerMoveVev.x += 1.f;
		}
		e->x += playerMoveVev.x;
		e->y += playerMoveVev.y;
	}
	else if (auto building = GET_ENTITY(e, building))
	{
		GetGameState(core);
		DefineInput(core);

		building->timer += core->deltaTime;

		if (building->timer > 10)
		{
			printf("building troop!\n");
			building->timer = 0.f;
			newEntity(e->x + Random::floatInRange(-25.f, 25.f), e->y + Random::floatInRange(-25.f, 25.f), Entity_unit, gameState);
		}
	}
}

void r(Entity *e, EngineCore* core)
{
	if (auto entity = GET_ENTITY(e, building))
	{
		core->spriteBatch->draw(glm::vec4{ e->x, e->y, 40, 40 }, glm::vec4{ 0.f, 0.f, 1.0f, 1.0f }, 3, 1.0f);
	}
	else if (auto entity = GET_ENTITY(e, unit))
	{
		core->spriteBatch->draw(glm::vec4{ e->x, e->y, 40, 40 }, glm::vec4{ 0.f, 0.f, 1.0f, 1.0f }, 3, 1.0f);
	}
	else if (auto entity = GET_ENTITY(e, script))
	{
		// debug rect
		entity->hitbox.DrawRect();
	}
	else if (auto entity = GET_ENTITY(e, player))
	{
		GetGameState(core);
	}
}

struct AssetFileInfo
{
	int musicCount;
	int textureCount;
	int effectCount;
};

// 300ms nopeampi compile !!!
#if EXE_COMPILE
typedef unsigned long DWORD, *PDWORD, *LPDWORD;
typedef struct _FILETIME {
	DWORD dwLowDateTime;
	DWORD dwHighDateTime;
} FILETIME, *PFILETIME;
#endif
