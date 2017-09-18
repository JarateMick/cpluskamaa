#include "game.h"

#include <chrono>
#include <string>
#include <vector>
// #define EXE_COMPILE 1
#include <unordered_set>
#include "Random.cpp"
#include <algorithm>
// #define _USE_MATH_DEFINES

#include <algorithm>
#include <lua.hpp>

#undef max
#undef min

using namespace std;
StackAllocator g_singleFrameAllocator;


ImageData::ImageData(const char* filename)
{
	surface = IMG_Load(filename);
	if (!surface)
		printf("IMG_Load: %s\n", IMG_GetError());
}

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



// TODO: SAVING LOADING!!!
//void SaveAllEntitys(game_state* gameState, const char* fileName)
//{
//	ALLEGRO_FILE* file = nullptr;
//
//	file = al_fopen(fileName, "wb");
//	if (!file)
//	{
//		printf("couldn't not open file");
//		ASSERT(false);
//	}
//
//	for (auto &Entity : gameState->entities)
//	{
//		al_fwrite(file, &Entity, sizeof(Entity));
//	}
//	al_fclose(file);
//}
//
//void LoadAllEntity(game_state* gameState, const char* fileName)
//{
//	ALLEGRO_FILE* file = al_fopen(fileName, "rb");
//
//	int64_t nb_read_total = 0, nb_read = 1;
//
//	if (file)
//	{
//		al_fseek(file, 0, ALLEGRO_SEEK_END);
//		int64_t fileSize = al_ftell(file);
//		al_fseek(file, 0, ALLEGRO_SEEK_SET);
//
//		int index = 0;
//
//		while (nb_read_total < fileSize && nb_read != 0)
//		{
//			Entity entity;
//			nb_read = al_fread(file, &entity, sizeof(Entity));
//			nb_read_total += nb_read;
//
//			ASSERT(index < ArrayCount(gameState->entities));
//			gameState->entities[index++] = entity;
//		}
//		al_fclose(file);
//	}
//	else
//	{
//		printf("couldn't open file");
//	}
//	gameState->currentEntityCount = (int)((float)nb_read_total / (float)sizeof(Entity));
//}


Entity* GetFirstAvaibleEntity(game_state* state)
{
	ASSERT(state->currentEntityCount < ArrayCount(state->entities));
	return &state->entities[state->currentEntityCount++];
}

// pointer to pointer that nullifys other pointers 
// sounds too complicated
void RemoveEntity(int index, game_state* state)
{
#if 1
	state->entities[index].type = Entity_Invalid;
#else // meh
	state->currentEntityCount--;
	Entity* array = state->entities;
	memcpy(array + index, array + state->currentEntityCount, sizeof(Entity));
	(array + index)->guid = index;
#endif
	// state->entities[state->currentEntityCount] = 0;
}


// Game:
//   * Better selection for region select
//   * Risk style battle screen
//   *

// Engine:
//   * Hotloading variable tweak -> (from google sheets maybe?)
//   * Configs files
//   * Better solutions for -> . -> . 
//   * Take look on allegro shader stuff
//   *


void SaveAllGameState(game_state* gameState)
{
	FILE* file = fopen("gameState0", "wb");
	if (file)
	{
		fwrite(gameState, sizeof(game_state), 1, file);
		fclose(file);
	}
}


lua_State* L;
EXPORT void Loop(EngineCore* core)
{
	game_state *gameState = (game_state*)core->memory->permanentStorage;
	// core->cameraX = gameState->cameraX;
	// core->cameraY = gameState->cameraY;
	if (!core->memory->isInitialized)
	{
		gameState->arena.InitalizeArena(core->memory->permanentStorageSize - sizeof(game_state),
			(uint8_t *)core->memory->permanentStorage + sizeof(game_state));


		L = core->script.L;

		Debug::_Debugger = core->debugger; // hööh :-(

		// TODO: defaults some neat fileloading facilities would be cool
		gameState->cameraSpeed = 50.0f;

		// Initialize some npc's;
		// floats = PushArray(&gameState->arena, 10000000, float);



		core->memory->isInitialized = true;

		gameState->entities[0].type = Entity_ninja;
		gameState->entities[1].type = Entity_npc;

		int j = 0;
		for (int i = 0; i < (int)ArrayCount(gameState->entities); i++)
		{
			gameState->entities[i].guid = i;
			j++;
		}
		gameState->currentEntityCount = 2;


		// init some npc's
		Entity* e = GetFirstAvaibleEntity(gameState);
		e->type = Entity_script;
		auto script = GET_ENTITY(e, script);
		sprintf(script->message, "%s", "Morjesta"); // Luassa olisi kiva maaritella jutut
		script->hitbox = { 0, 0, 40, 80 };


		e = GetFirstAvaibleEntity(gameState);
		e->type = Entity_player;
		e->x = 150.f;
		e->y = 150.f;


		SDL_Surface* surface = core->resources.LoadSurface("europe.png");
		gameState->worldmap.provinces.surface = surface;

		surface = core->resources.LoadSurface("europedata.png");
		gameState->worldmap.visual.surface    = surface;
		gameState->worldmap.temptextureid = core->resources.SurfaceToGlTexture(surface);

		if (!Debug::restartLog())
		{
			printf("log restart failed: %s, %d", __FILE__, __LINE__);
		}
	}

	/*************************************************/
	// ENTITY UPDATE
	for (int i = 0; i < gameState->currentEntityCount; i++)
	{
		f(&gameState->entities[i], core);
	}

	// TODO: Nuke this init phase
	static bool init = false;
	if (!init) // || core->input->isKeyPressed(SDL_SCANCODE_SPACE))
	{
		Random::init();

		init = true;
	}

	// g_boxy.Update(core, gameState);

	// pelaajan kontrollit
	// TODO: Engine kamat engineen
	InputManager* input = core->input;

	if (input->isKeyPressed(SDL_SCANCODE_4))
	{
		SaveAllGameState(gameState);
	}

	static char *saveFile = "testailu.data";
	if (input->isKeyPressed(SDL_SCANCODE_1))
	{
		// SaveAllEntitys(gameState, "entity.sav");
	}
	else if (input->isKeyPressed(SDL_SCANCODE_2))
	{
		// LoadAllEntity(gameState, "entity.sav");
		lua_State* L = core->script.L;
		lua_getglobal(L, "LoadTileMap");
		lua_pushnumber(L, 100);

		if (lua_pcall(L, 1, 0, 0) != 0)
		{
			Debug::logError("what the fuck!");
			Debug::logError(lua_tostring(L, -1));
			fprintf(stderr, "%s\n", lua_tostring(L, -1));
			debugBreak();
		}
		lua_pop(L, 1);
	}



	static bool initted = false;
	static GLuint tid;
	if (!initted)
	{
		initted = true;
		// showToPlayer
		tid = core->resources.SurfaceToGlTexture(gameState->worldmap.visual.surface);
		// FreeTexture(&tid);
	}



	if (input->isKeyDown(SDL_SCANCODE_5))
	{
		Uint32 color = gameState->worldmap.provinces.GetPixel(input->mouse.x - 200.f, 600.f - input->mouse.y);
		std::cout << color << "\n";
	}

	if (input->isKeyDown(SDL_SCANCODE_6))
	{
		Uint32 targetcolor = gameState->worldmap.provinces.GetPixel(input->mouse.x - 200.f, 640.f - input->mouse.y);
		FloodFillImage(&gameState->worldmap.provinces, &gameState->worldmap.visual, input->mouse.x - 200.f, 640.f - input->mouse.y, targetcolor, Uint32(gameState->worldmap.editorColor));

		core->resources.FreeTexture(&tid);
		gameState->worldmap.temptextureid = core->resources.SurfaceToGlTexture(gameState->worldmap.visual.surface); 
	}
}






// hex sizes --- world in meters
static const float hexWidth = 30.f;
static const float hexHeight = 35.f;
static const float halfHexW = hexWidth / 2.f;
static const float halfHexH = hexHeight / 2.f;
static const float hexInMeters = 1.f;
static const float hexInPixels = hexWidth;
static const float metersToPixel = hexInPixels / hexInMeters;
static const float HexHeightInMeter = hexHeight / hexWidth;

// #define hexWidth 30.f
// #define hexheigth 35.f

struct Entity2
{
	void Update(int a)
	{
		this->x = a;
		this->y = a;
	}
	void Printf(int message)
	{
		printf("hello from lua %d", message);
	}

	float x;
	float y;

	void Draw()
	{
		// static ALLEGRO_BITMAP* bmp = al_load_bitmap("test.png");
		// al_draw_bitmap(bmp, x, y, 0);
	}


	void SetPos(float x, float y)
	{
		this->x = x;
		this->y = y;
	}
};


game_state* state = 0;
static int ID = -1;
int CreateEntityHandle()
{
	Entity &e = state->entities[++ID];
	e.x = 0.f;
	e.y = 0.f;
	e.type = Entity_script;
	e.guid = ID;
	return ID;
}

void SetEntityPosition(float x, float y, int id)
{
	Entity* e = &state->entities[id];
	e->x = x;
	e->y = y;
}

void SetEntityVel(float velX, float velY, int id)
{
	state->entities[id].velX = velX;
	state->entities[id].velY = velY;
}

// poistossa synccaa molemmat swap and win !
// serialisoi     
// id -> entity formaattiin luan sisällä

//void SetAllEntitys()
//{
//	sol::table entity = (*luaP)["ents"]; // func
//	for (int i = 0; i < ID; i++)
//	{
//		Entity& e = state->entities[i];
//		e.x = entity[i]["x"] = e.x + e.velX;
//		e.y = entity[i]["y"] = e.y + e.velY;
//	}
//}

//void SetAllEntitys2(int count, const char* table)
//
//{
//	sol::table entity = (*luaP)[table]; // func
//	for (int i = 1; i < count; i++)
//	{
//		Entity& e = state->entities[i];
//		e.x = entity[i]["x"] = e.x + e.velX;
//		e.y = entity[i]["y"] = e.y + e.velY;
//	}
//}

EXPORT __declspec(dllexport) typedef struct
{
	float x;
	float y;
} v2;

EXPORT __declspec(dllexport) void SetTile(int x, int y, int id)
{
}

const char* entitySetFunctionName = "foo2";
EXPORT __declspec(dllexport) void Physics(v2* data, int count)
{
	// static ALLEGRO_BITMAP* bmp = al_load_bitmap("test.png");

	for (int i = 0; i < ID + 1; i++)
	{
		Entity& e = state->entities[i];
		data[i].x += e.velX;
		data[i].y += e.velY;
	}

	for (int i = 0; i < ID + 1; i++)
	{
		// al_draw_bitmap(bmp, data[i].x, data[i].y, 0);
	}
}

void SetAllEntitysPositions()
{
	// (*luaP)[entitySetFunctionName]();
}

void DrawAllEntitys(EngineCore* core)
{
	// static ALLEGRO_BITMAP* bmp = al_load_bitmap("test.png");

	// al_hold_bitmap_drawing(true);
	for (int i = 0; i < ID; i++)
	{
		Entity& e = state->entities[i];
		// al_draw_bitmap(bmp, e.x - core->cameraX, e.y - core->cameraY, 0);
	}
	// al_hold_bitmap_drawing(false);
}

//inline void my_panic(sol::optional<std::string> maybe_msg) {
//	// std::cerr << "Lua is in a panic state and will now abort() the application" << std::endl;
//	if (maybe_msg) {
//		const std::string& msg = maybe_msg.value();
//		// std::cerr << "\terror message: " << msg << std::endl;
//	}
//	// When this function exits, Lua will exhibit default behavior and abort()
//}

// struct Vector2
// {
	// float x, y;
// };

EXPORT __declspec(dllexport) void SyncData(v2* data, int count)
{
	for (int i = 0; i < count; i++)
	{
		v2* d = &data[i];
		d->x = 10000;
		d->y = 10000;
	}
}

EXPORT __declspec(dllexport) void Hello2(v2* data, int count)
{
	for (int i = 0; i < count; i++)
	{
		v2* d = &data[i];
		d->x = i;
		d->y = i;
	}
}

EXPORT __declspec(dllexport) void Dangerous(v2* data, int count)
{
}

EXPORT __declspec(dllexport) void EmptyFunc(double x, double y)
{

}

EXPORT __declspec(dllexport) int GetX(int id)
{
	return 1;
}
EXPORT __declspec(dllexport) int GetY(int id)
{
	return 2;
}

EXPORT __declspec(dllexport) void Swapper(float &x, float &y, int id)
{
	x = 10;
	y = 10;
}

EXPORT __declspec(dllexport) void Drawer(float x, float y)
{
	// static ALLEGRO_BITMAP* texture = al_load_bitmap("test.png");
	// al_draw_bitmap(texture, x, y, 0);
}

void Hello(void* vecs)
{
	// printf("tanne tuli %i!", vecs[0].x);
	// v2* v = (v2*)vecs;
	// printf("hei %i", v[1].x);
}


void LuaErrorWrapper(game_state* state, EngineCore* core)
{
	InputManager* input = core->input;



	//	int succ = luaL_loadfile(L, filename);
	lua_getglobal(L, "main_function");
	int ret = lua_pcall(L, 0, 0, 0);
	if (ret != 0)
	{
		fprintf(stderr, "%s\n", lua_tostring(L, -1));

		debugBreak();
	}
	lua_settop(L, 0);

	//if (!init1)
	//{
	//	core->script.luaP = &lua;
	//	lua.open_libraries(sol::lib::base, sol::lib::os, sol::lib::string, sol::lib::math, sol::lib::ffi,
	//		sol::lib::package, sol::lib::jit, sol::lib::table);
	//	lua["Draw"] = DrawRect2;
	//	lua["HoldBitmap"] = al_hold_bitmap_drawing;
	//	init1 = true;

	//	// lua["outoStruct"] = &Hello;
	//	// lua["e2"] = e2;
	//	// lua["Update"] = &Entity2::Update;
	//	// lua["DrawLots"] = &DrawLotsOfEntitys;
	//	// lua["DrawUpdate"] = &DrawUpdateEntity;
	//	// void SetAllEntitys2(int count, const char* table)
	//	// lua["SetAllEntity2"] = &SetAllEntitys2;
	//	//usertype<Entity2> Entity2Type(
	//	//	"Update", &Entity2::Update,
	//	//	"Printf", &Entity2::Printf
	//	//);
	//	//lua.set_usertype<Entity2>("Entity2", Entity2Type);
	//	// lua.set("DrawUpdate2", sol::c_call<sol::wrap<decltype(&DrawUpdateEntity), &DrawUpdateEntity>>);
	//	//lua.new_usertype<Entity2>("Entity2",
	//	//	"Update", &Entity2::Update,
	//	//	"Printf", &Entity2::Printf,
	//	//	"x", &Entity2::x,
	//	//	"y", &Entity2::y,
	//	//	"SetPos", &Entity2::SetPos,
	//	//	"Draw", &Entity2::Draw
	//	//	);
	//	// lua.script("ents = {}");
	//	lua["SetEntityPos"] = &SetEntityPosition;
	//	lua["SetEntityVel"] = &SetEntityVel;
	//	lua["CreateEntityHandle"] = &CreateEntityHandle;
	//	// lua["input"] = sol::table;
	//	lua.create_named_table("Input");
	//	lua["Input"]["mouse"] = lua.create_table_with();
	//	lua.create_named_table("time");

	//	lua["PhysAndDraw"] = &Physics;

	//	luaP = &lua;
	//}

	//(*luaP)["Input"]["mouse"]["x"] = input->mouseX;
	//(*luaP)["Input"]["mouse"]["y"] = input->mouseY;

	//(*luaP)["Input"]["w"] = input->isKeyDown(SDL_SCANCODE_W);
	//(*luaP)["Input"]["s"] = input->isKeyDown(SDL_SCANCODE_S);
	//(*luaP)["Input"]["d"] = input->isKeyDown(SDL_SCANCODE_D);
	//(*luaP)["Input"]["a"] = input->isKeyDown(SDL_SCANCODE_A);
	//(*luaP)["time"]["dt"] = core->deltaTime;

	//al_hold_bitmap_drawing(true);

	//try
	//{
	//	lua.safe_script_file("I:/Dev/Allegro/allegro/bin/lua/main2.lua");
	//}
	//catch (sol::error& err)
	//{
	//	printf("sol::error: %s", err.what());
	//	// render
	//	static ALLEGRO_FONT* font = al_load_ttf_font("rs.ttf", 30, 0);
	//	al_draw_text(font, al_map_rgb(255, 120, 120), 40, 40, 0, err.what());

	//	// al_flip_display();

	//	// core->forceFlip();

	//	DebugBreak();
	//}
	//catch (...)
	//{
	//	DebugBreak();
	//}

	//al_hold_bitmap_drawing(false);
}

EXPORT void Draw(EngineCore* core)
{
	game_state *gameState = (game_state*)core->memory->permanentStorage;
	state = gameState;
	DefineInput(core);

	//SDL_GL_MakeCurrent(core->window, *core->glcontext);
	// TODO: Free camera from main
	float cameraSpeed = gameState->cameraSpeed * core->deltaTime; // ei toimi samalla tavalla kuin updaten mov
	if (input->isKeyDown(SDL_SCANCODE_W))
	{
		core->camera2D->setPosition(core->camera2D->getPosition() + glm::vec2{ 0, cameraSpeed });
	}
	if (input->isKeyDown(SDL_SCANCODE_S))
	{
		core->camera2D->setPosition(core->camera2D->getPosition() + glm::vec2{ 0, -cameraSpeed });
	}
	if (input->isKeyDown(SDL_SCANCODE_A))
	{
		core->camera2D->setPosition(core->camera2D->getPosition() + glm::vec2{ -cameraSpeed, 0 });
	}
	if (input->isKeyDown(SDL_SCANCODE_D))
	{
		core->camera2D->setPosition(core->camera2D->getPosition() + glm::vec2{ cameraSpeed, 0 });
	}

	// _getglobal(L, "main_function");
	// lua_pcall(L, 0, 0, 0);

	core->spriteBatch->draw(glm::vec4{ 200.f, 0.f, 528.f, 640.f }, glm::vec4{ 0.f, 0.f, 1.0f, 1.0f }, gameState->worldmap.temptextureid, 1.0f);

	DrawAllEntitys(core);

	// render all entitys
	for (int i = 0; i < gameState->currentEntityCount; i++)
	{
		Entity* e = &gameState->entities[i];
		r(e, core);
	}

	// TODO: Fontit resource managerille
}

