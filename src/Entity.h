#pragma once

#define EXTEXP extern "C" __declspec(dllexport)

#ifndef EXPORT
#define EXPORT extern "C" 
#endif

// CollisionBody:
//	create();
//	destroy();

//
//  x, y, w, h, ... r
//  type: circle, square
//  
//  queries: 
// 

enum Entity_Enum
{
	Entity_Invalid,
	Entity_ninja,
	Entity_npc,

	Entity_unit,

	Entity_script,

	Entity_player,

	Entity_building,

	Entity_bullet,

	Entity_MAX,
};

const char *EntityNames[Entity_MAX] =
{
	"Invalid",
	"ninja",
	"npc",
	"unit",
	"script",
	"player",
	"building",
	"bullet",
};

// oli

struct Rect
{
	float x, y, w, h;

	bool Contains(float x, float y)
	{
		return (this->x < x &&  this->x + (w) > x &&
				this->y  < y && this->y + (h) > y);
	}

	void UseLeftBottomAsStart()
	{
		if (w < 0)
		{
			w  = std::abs(w);
			x -= w;
		}
		if (h < 0)
		{
			h  = std::abs(h);
			y -= h;
		}
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

// hello


struct Entity
{
	Entity() {} // = default;
	~Entity() = default;

	Entity_Enum type;
	uint32_t	guid;

	// pos
	// glm::vec2 pos;
	float x, y;
	float velX, velY;
	bool alive;

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
		struct Unit
		{
			int damage;
			int moveSpeed;
			int strength;
			Uint32 side;

			Uint32 lastFrameProv;

			int targetX, targetY;
			int originalTargetX, originalTargetY;

			Entity* attackTarget;

			float mainAttackCD;
			float attackRange;

			std::vector<int> path; // LEAK!

			// attack speed jne...
			// (instant attacks melee) <-> (type?)
			// if in range attack jne...
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

			Rect selectionRect;
			bool selectingTroops;
		} player;
		struct
		{
			building_type type;
			Uint32        side;
			float         timer;
			GLbyte        textureId;
			glm::vec4     textureUv;
		} building;
		struct // TODO: (optimointi) bulletit voisi laittaa omaksi arrayksi koska paljon pienimpi
		{
			// int exparationFrame;  // laske speed/frame ja vertaile tata laskemisen sijasta ?
			float speed;

			float rangeSquared;
			float startX, startY;
		} bullet;
	};
};

struct EngineCore;
struct game_state;

void f(Entity *e, EngineCore* core);
void r(Entity *e, EngineCore* core);

Entity* GetFirstAvaibleEntity(game_state* state);
EXPORT __declspec(dllexport) Entity* newEntity(float x, float y, Entity_Enum type, game_state* state); //LUA_E_F
bool buildBuilding(float x, float  y, building_type type, game_state* state, Uint32 side);

EXPORT __declspec(dllexport) Entity* getById(int i, void* gameState);



//std::vector<int> findPath(int startId, int goalId, game_state* gameState);




#define GET_ENTITY(e, entity_type) ((e)->type == Entity_##entity_type ? &(e)->entity_type : 0)
#define ENT(e, entity_type)        &((e)->entity_type)

