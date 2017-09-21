#pragma once

enum Entity_Enum
{
	Entity_Invalid,
	Entity_ninja,
	Entity_npc,

	Entity_unit,

	Entity_script,

	Entity_player,

	Entity_building,

	Entity_MAX
};

const char *EntityNames[Entity_MAX] =
{
	"Invalid",
	"ninja",
	"npc",
	"unit",
	"script",
	"player",
	"building"
};

// #include "glm/vec2.hpp"
// #include "glm/geometric.hpp"
// #include "glm/glm.hpp"
//
//

#include "gl/SpriteBatch.h"
struct Rect
{
	float x, y, w, h;

	bool Contains(float x, float y)
	{
		return (this->x < x && this->x + w > x &&
			this->y  < y && this->y + h > y);
	}

	void DrawRect()
	{
		Debug::drawBox(x, y, w, h);
	}
};

enum building_type
{
	building_none,
	building_millitary_factory,
	building_mill,
	building_max,
};

struct Entity
{
	Entity_Enum type;
	uint32_t	guid;

	// pos
	// glm::vec2 pos;
	float x, y;
	float velX, velY;

	// vel
	// acc

	union
	{
		struct
		{
			int damage;
		} ninja;
		struct
		{
			int damage;
		} npc;
		struct
		{
			int damage;
			int moveSpeed;
			int strength;
			Uint32 side;
		} unit;
		struct
		{
			Rect hitbox;
			char message[16];
		} script;
		struct
		{
			int currentRoom;

			Uint32 side;
			// currently Selected entities! pointer
			int cash;

			building_type selectedBuildingType;
		} player;
		struct
		{
			building_type type;
			Uint32        side;
			float         timer;
		} building;
	};
};

struct EngineCore;
struct game_state;
void f(Entity *e, EngineCore* core);
void r(Entity *e, EngineCore* core);

Entity* GetFirstAvaibleEntity(game_state* state);
Entity* newEntity(float x, float y, Entity_Enum type, game_state* state);
bool buildBuilding(float x, float  y, building_type type, game_state* state, Uint32 side);

#define GET_ENTITY(e, entity_type) ((e)->type == Entity_##entity_type ? &(e)->entity_type : 0)
#define ENT(e, entity_type)        &((e)->entity_type)

