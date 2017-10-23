#pragma once
// #include "game.h"
#include <glm/glm.hpp>
#include <vector>

struct GridPosition;
constexpr int MAXIUM_BULLETS = 10000;
constexpr int cellSize = 48;      // 590 x 480        5900        x        4800
const int mapSizeMultiplier = 30;
constexpr float NODE_MULTIPLIER = 1.5f;
constexpr int CellsX = 2 * (int(5900 * NODE_MULTIPLIER) / cellSize) + 1 + 3;
constexpr int CellsY = 2 * (int(4800 * NODE_MULTIPLIER) / cellSize) + 1 + 3;

struct v2 { int x, y; };

typedef glm::vec2 vec2f;
struct PhysicsBody
{
	float x, y;       
	float r;
	int   owner;
};

struct BulletBody
{
	vec2f   position;       
	float   r;
	Uint32  side;
};

struct BulletStart
{
	vec2f    position; 
	float rangeSqrt;
};

struct Bullets
{
	BulletStart* start;
	vec2f*       accelerations;
	BulletBody*  bodies;
};

struct SpatialHash            // map width = textureW * 10, textureH * 10
{
	// hash map :(
	std::vector<PhysicsBody*> hashMap[CellsY][CellsX];
} hash4r;

static inline PhysicsBody* getBody(int id, PhysicsBody* bodies);

void AddBodyToGrid(PhysicsBody* body, SpatialHash* hash, GridPosition* positions);
void SwapBody(int gridX, int gridY, int newGridX, int newGridY, PhysicsBody* bodyToSwap, SpatialHash* hash);
void UpdateAllGridPosition(GridPosition* positions, PhysicsBody* bodies, SpatialHash* hasher, const int count);

void allUniques(int startY, int endY, int startX, int endX, SpatialHash* hash, PhysicsBody* bodiesOut[], int outSize);

void CheckCollision(int start, std::vector<PhysicsBody*>* bodies, PhysicsBody* __restrict body);
int CheckCollisions(SpatialHash* hash);

void circleCollision(PhysicsBody* a, PhysicsBody* b);
bool circleCollision(BulletBody* __restrict a, PhysicsBody* __restrict b);
void circleCollision(PhysicsBody* __restrict a, PhysicsBody* __restrict b);

void updateCollision(PhysicsBody* bodies, int count);
int simulateBullets(BulletBody* bodies, vec2f* bulletAcceceration, BulletStart* start, int count);

void renderPhysicsBodies(PhysicsBody* bodies, int count);

void swapAll(int i, int count, Bullets* bullets);
int collideBullets(SpatialHash* hash, BulletBody* bulletBodies, Uint32* sides, int size, Bullets* bullets, std::vector<int>* damageOut);

inline v2 HashPoint(int x, int y);
inline v2 HashPoint(v2 v);
