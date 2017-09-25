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
#include <SDL2\SDL_image.h>

#include <cstdint>
#include <vector>
#include <functional>
#include <math.h>
#define CLAMP(x, upper, lower) (std::min(upper, std::max(x, lower)))

#include "TextureHolder.h"
#include "fileSystem.h"

// #include "Hex.h"
#define Internal static

struct game_state; 

// TODO: Vec2  -> typedeffaa vec jne....
// oma vs Glmstruct
// oma: 
//  + nopeampi compileta
//  + vecit helppoja tehdä
// glm: 
//   + mat4 voi olla ärsyttävä tehdä itse


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

const int mapSizeMultiplier = 4;
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

introspect("game_state: hello world") struct game_state
{
	memory_arena   arena;
	Entity         entities[10000];
	Entity*        player;
	Entity**       selectedEntitys;  // oma ^^ areenaan allokoiva array
	int            selectedCount;
	int            maxSelected;

	int            currentEntityCount;
	ProvinceData   provinceData;

	// TileMap tilemap;
	// TilemapEditor editor;
	WorldMap worldmap;

	float cameraSpeed;

	bool dirtyFlag;
};

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
