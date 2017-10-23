#pragma once
#define EXPORT extern "C" 
// __declspec( dllexport )
#define introspect(name)

#define kiloBytes(value) ((value)*1024LL)
#define megaBytes(value) (kiloBytes(value)*1024LL)
#define gigaBytes(value) (megaBytes(value)*1024LL)
#define teraBytes(value) (gigaBytes(value)*1024LL)
#define ArrayCount(array) ( sizeof(array) / sizeof((array)[0]))

#include "Debug.h"
#include "Random.cpp"
#include "graph.h"
#include "../Include/SDL2/SDL.h"

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


struct v2 { int x, y; };


#include <lua.hpp>
// #include <SDL2\SDL_image.h>
struct SpatialHash;
struct PhysicsBody;

inline v2 HashPoint(int x, int y);
void allUniques(int startY, int endY, int startX, int endX, SpatialHash* hash, PhysicsBody* bodiesOut[], int outSize);
static inline PhysicsBody* getBody(int id, PhysicsBody* bodies);

#include <cstdint>
#include <vector>
#include <functional>
// #include <math.h>
#define CLAMP(x, upper, lower) (std::min(upper, std::max(x, lower)))

#include "TextureHolder.h"
#include "fileSystem.h"

// #include "Hex.h"
#define Internal static

typedef glm::vec2 vec2f;
struct game_state;

// TODO: Vec2  -> typedeffaa vec jne....
// oma vs Glmstruct
// oma: 
//  + nopeampi compileta
//  + vecit helppoja tehdä
// glm: 
//   + mat4 voi olla ärsyttävä tehdä itse

#define NAME(ending) (func_timing##ending)

#define START_TIMING() \
	auto NAME(1)(std::chrono::high_resolution_clock::now()); \

#define END_TIMING() \
	auto NAME(2)(std::chrono::high_resolution_clock::now());	\
	auto elapsedTime(NAME(2) - NAME(1));						\
	printf("Time: %f\n", std::chrono::duration_cast<std::chrono::duration<float, std::milli>>(elapsedTime).count()  );

#define START_TIMING2() \
	auto NAME(3)(std::chrono::high_resolution_clock::now()); \

#define END_TIMING2() \
	auto NAME(4)(std::chrono::high_resolution_clock::now());	\
	auto elapsedTime2(NAME(4) - NAME(3));						\
	printf("Time: %f\n", std::chrono::duration_cast<std::chrono::duration<float, std::milli>>(elapsedTime2).count()  );


struct PhysicsBody
{
	float x, y;       // 16  ->  20  ->  24 | render id
	float r;
	int owner;
};


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

// color / image data
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
		return ((unsigned int*)surface->pixels)[y*(surface->pitch / sizeof(unsigned int)) + x];
	}
};


struct FileWatcher
{
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

	void showImgui(ResourceType type) { }
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
			// fprintf(stderr, "%s\n",/ lua_tostring(L, -1));
			sprintf(buffer, "[error]: %s\n", lua_tostring(L, -1));
			printf("%s", buffer);
			return buffer;
			// debugBreak();
		}
		lua_settop(L, 0);

		return 0;
	}
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
#define PushArray2(Arena, Count, Size) PushSize_(Arena, (Count)*Size)

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
#include "core.h"

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
//	else
//	{
//return;
//	}
}

struct WorldMapEditor
{
	Uint32 editorColor;
	int inputProvinceId;
	float inputX;
	float inputY;
};




const int mapSizeMultiplier = 30;
constexpr float NODE_MULTIPLIER = 1.5f;
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

	Uint32 GetSideUnderMouse(const glm::vec2* mouse)
	{
		auto mousePos = GetMouse(mouse);
		Uint32 color = provinces.GetPixel(mousePos.x, mousePos.y); // real size 
		return color;
	}

	glm::vec2 GetMouse(const glm::vec2* mouse)
	{
		float mx = (mouse->x - dimensions.x) / mapSizeMultiplier;
		float my = (dimensions.w - mouse->y) / mapSizeMultiplier;   // map->dimensions.w / 3 - input->mouse.y / 3;
		return { mx, my };
	}

	Uint32 GetPixelSideFromWorld(int x, int y)
	{
		return provinces.GetPixel(x / mapSizeMultiplier, 480.f - y / mapSizeMultiplier); // real size 
	}

	Uint32 GetCurrentHolder(int x, int y)
	{
		return visual.GetPixel(x / mapSizeMultiplier, 480.f - y / mapSizeMultiplier);
	}

	Uint32 GetPixelSide(int x, int y)
	{
		// contains ?
		return provinces.GetPixel(x, y); // real size 
	}

	void changeSideWorld(int x, int y, Uint32 to, EngineCore* core)
	{
		x = (x - dimensions.x) / mapSizeMultiplier;
		y = (dimensions.w - y) / mapSizeMultiplier;
		Uint32 targetColor = GetPixelSide(x, y);

		SDL_PixelFormat *fmt = provinces.surface->format;
		Uint8 alpha = getAlpha(targetColor, fmt);
		if (targetColor != 0xFF000000 && alpha != 0)
		{
			FloodFillImage(&provinces, &visual, x, y, targetColor, to);

			// taman jos tekee framen lopussa niin ei tarvitse tehda kuin yksi tekstruuri generaatio
			UpdateTexture(core);
		}
	}

	void UpdateTexture(EngineCore* core)
	{
		core->resources.FreeTexture(&temptextureid);
		temptextureid = core->resources.SurfaceToGlTexture(visual.surface);
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

struct ProvinceEditorData
{
	std::vector<int> selectedNeighbours;
	int           selectedProvinceId;
};

struct PathFindingUi
{
	int  startId, endId;
	bool drawPath;
};

std::vector<int> getAllProvinceNeighbours(int id);

introspect("hello") struct MapNode
{
	int   id;
	float x, y; // for rendering and debugging 
};
// Graph<MapNode, int> nodes(64);

struct BulletBody
{
	vec2f      position;       // 16  ->  20  ->  24 | render id
	float   r;
	Uint32  side;
};

struct BulletStart
{
	vec2f    position;
	float rangeSqrt;
};

constexpr int maxiumBullets = 10000;

// resolve collision -> damages -> bullets

struct ThreadSharedData
{
	PhysicsBody*   thisFrame;
	PhysicsBody*   lastFrame;
};


#include "Animation.h"


constexpr int cellSize = 48;      // 590 x 480        5900        x        4800
constexpr int CellsX = 2 * (int(5900 * NODE_MULTIPLIER) / cellSize) + 1 + 3;
constexpr int CellsY = 2 * (int(4800 * NODE_MULTIPLIER) / cellSize) + 1 + 3;
struct SpatialHash            // map width = textureW * 10, textureH * 10
{
	// hash map :(
	std::vector<PhysicsBody*> hashMap[CellsY][CellsX];
} hash4r;

struct GridPosition { int x, y; };
static GridPosition gridPositions[MAX_ENTITY_COUNT];
// BODY
void AddBodyToGrid2(PhysicsBody* body, SpatialHash* hash, GridPosition* positions);

struct UnitStructure
{
	// typet
	// target
	Entity* attackTargets[MAX_ENTITY_COUNT];
	attack_type attackTypes[MAX_ENTITY_COUNT];
};


struct Province
{
	Uint32 side;
};


#define I
introspect("game_state:") struct game_state
{
	Entity         entities[MAX_ENTITY_COUNT];
	Entity*        player;
	Entity**       selectedEntitys;  // oma ^^ areenaan allokoiva array

	int            selectedCount;
	int            maxSelected;

	PhysicsBody*   bodies;
	GridPosition*  gridPosition;
	Uint32         allSides[MAX_ENTITY_COUNT];
	BulletBody     bulletBodies[maxiumBullets];
	BulletStart    bulletStart[maxiumBullets];
	vec2f          BulletAccelerations[maxiumBullets];

	ThreadSharedData threadShared;
	Animations       unitAnimations;

	UpiEngine::ColorRGBA8 entityColors[MAX_ENTITY_COUNT];

	SpatialHash*   spatialGrid;

	int            bulletCount;
	int            currentEntityCount;

	ProvinceData   provinceData;
	Province       provinces[MAX_ENTITY_COUNT];

	ZombieAi       ai;
	UnitStructure  unitStructure;



	// TileMap tilemap;
	// TilemapEditor editor;
	WorldMap worldmap;

	float cameraSpeed;

	bool dirtyFlag;                  // joku oma homma näille
	ProvinceEditorData provinceEditor;
	PathFindingUi      pathfindingUi;

	memory_arena   arena;

	// fuck
	I Graph<MapNode, int>* MapNodes;
	I std::vector<int>(*getAllProvinceNeighbours)(int);
	I void(*newNode)(int index, int id, float x, float y);
};

//#define GetTarget(int id, game_state* gameState) gameState->unitStructure.attackTargets[id]
//
static inline Entity* getTargets(int id, game_state* gameState)
{
	return gameState->unitStructure.attackTargets[id];
}

static inline void setTargets(int id, game_state* gameState, Entity* target)
{
	gameState->unitStructure.attackTargets[id] = target;
}

static inline attack_type getAttackType(int id, game_state* gameState)
{
	return gameState->unitStructure.attackTypes[id];
}

void SetAnimation(game_state* gameState, int guid, Anim_enum animType);
void InitAnimation(game_state* gameState, int guid)
{
	SetAnimation(gameState, guid, Anim_Archer_Run_Right);
}

void SetAnimation(game_state* gameState, int guid, Anim_enum animation)
{
	gameState->unitAnimations.animationTypes[guid] = Anim_type_looping;
	gameState->unitAnimations.animations[guid] = animation;
	gameState->unitAnimations.currentAnimationFrame[guid] = 0;
	gameState->unitAnimations.frameTimes[guid] = 1;

	// TODO: korjaa
	gameState->unitAnimations.uvs[guid] = { 0.f, 0.f, 0.08f, 1.0f }; // default
}

void SwapAnims(game_state* gameState, int toGuid, int fromGuid)
{
	Animations* anims = &gameState->unitAnimations;

	anims->frameTimes[toGuid]            = anims->frameTimes[fromGuid];
	anims->animationTypes[toGuid]        = anims->animationTypes[fromGuid];
	anims->animations[toGuid]            = anims->animations[fromGuid];
	anims->currentAnimationFrame[toGuid] = anims->currentAnimationFrame[fromGuid];
	anims->uvs[toGuid]                   = anims->uvs[fromGuid];

    // SetAnimatio
}

inline int GetColorToId(game_state* state, Uint32 color)
{
	auto map = state->provinceData.colorToId;
	auto iter = map->find(color);
	if (iter != map->end())
	{
		return iter->second;
	}
	else
	{
		// __debugbreak();
		// ASSERT(false);
		return -1;
	}
}

std::vector<int> BreadthFirst(int startID, Graph<MapNode, int>* graph, int goalId);

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
} FILETIME, *PFILETIME; ]
#endif
