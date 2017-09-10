#pragma once

enum Entity_Enum
{
	Entity_Invalid,
	Entity_ninja,
	Entity_npc,
	Entity_unit,
	Entity_script,

	Entity_player,

	Entity_MAX
};

const char *EntityNames[Entity_MAX] =
{
	"Invalid",
	"ninja",
	"npc",
	"unit",
	"script",

	"player"
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
	// radius
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
		} unit;
		struct
		{
			Rect hitbox;
			char message[16];
		} script;
		struct
		{
			int currentRoom;
		} player;
	};
};

#define GET_ENTITY(e, entity_type) ((e)->type == Entity_##entity_type ? &(e)->entity_type : 0)
#define ENT(e, entity_type)        &((e)->entity_type)
