#include "game.h"

#include <chrono>
#include <string>
#include <vector>
#include <iostream>

#include "Entity.cpp"
#include "fileSystem.cpp"
#include "meta.cpp"
#include "graph.h"
#include "util/collection_types.h"
#include "util/array.h"
#include "util/memory.cpp"

#include <lua.hpp>
#include <SDL2/SDL_image.h>

#undef max
#undef min 

#define NAME(ending) (func_timing##ending)

#define START_TIMING() \
	auto NAME(1)(std::chrono::high_resolution_clock::now()); \

#define END_TIMING() \
	auto NAME(2)(std::chrono::high_resolution_clock::now());	\
	auto elapsedTime(NAME(2) - NAME(1));						\
	printf("Time: %f\n", std::chrono::duration_cast<std::chrono::duration<float, std::milli>>(elapsedTime).count()  );




// preprocessor
// oma lispy> 
// ota paikkasi auringon alta
// 
// fysiikka bodyja      -->      suurin osa varmaankin ympyröitä
// 
// ratkaise kaikki collisionit:
// 
// laita kaikki targetit oikein
// 
// addbody(      );
// removebody(   );
// 
// mahdollisuus raycastata

// TODO: 
//   * fysiikka jutut: spatial hash, quad tree
//   * 
//   * 

struct PhysicsBody
{
	float x, y;       // 16  ->  20  ->  24 | render id
	float r;
	int owner;
};

void circleCollision(PhysicsBody* a, PhysicsBody* b);
// 

constexpr int cellSize = 32;      // 590 x 480        5900        x        4800
constexpr int CellsX = (5900) / cellSize + 1;
constexpr int CellsY = (4800) / cellSize + 1;

struct SpatialHash            // map width = textureW * 10, textureH * 10
{
	// hash map :(

	std::vector<PhysicsBody*> hashMap[CellsY][CellsX];
} hash4r;

void debugDraw(SpatialHash* hash)
{
	for (int i = 0; i < CellsY; i++)
	{
		for (int j = 0; j < CellsX; j++)
		{
			glm::vec4 box(j * cellSize, i * cellSize, cellSize, cellSize);
			Debug::drawBox(box);
		}
	}
}

void initSpatial(SpatialHash* hash)
{
	for (int i = 0; i < CellsY; i++)
	{
		for (int j = 0; j < CellsX; j++)
		{
			hash->hashMap[i][j].reserve(256);
			// hash->hashMap[i][j].clear();
		}
	}
}

void clearSpatial(SpatialHash* hash)
{
	for (int i = 0; i < CellsY; i++)
	{
		for (int j = 0; j < CellsX; j++)
		{
			hash->hashMap[i][j].clear();
		}
	}
}

inline v2 HashPoint(int x, int y)
{
	return { x / cellSize, y / cellSize };
}

inline v2 HashPoint(v2 v)
{
	return { v.x / cellSize, v.y / cellSize };
}

void AddBodyToGrid(PhysicsBody* body, SpatialHash* hash)
{
	v2 min{ (int)body->x - (int)body->r, (int)body->y - (int)body->r };
	v2 max{ (int)body->x + (int)body->r, (int)body->y + (int)body->r };
	v2 point1 = HashPoint(min.x, min.y);
	v2 point2 = HashPoint(min.x, max.y);
	v2 point3 = HashPoint(max.x, min.y);
	v2 point4 = HashPoint(max.x, max.y);

	hash->hashMap[point1.y][point1.x].push_back(body);
	hash->hashMap[point2.y][point2.x].push_back(body);
	hash->hashMap[point3.y][point3.x].push_back(body);
	hash->hashMap[point4.y][point4.x].push_back(body);
}

int CheckCollisions(SpatialHash* hash)
{
	int count = 0;
	for (int i = 0; i < CellsY; i++)
	{
		for (int j = 0; j < CellsX; j++)
		{
			std::vector<PhysicsBody*>* v = &hash->hashMap[i][j];
			if (!v->empty())
			{
				std::sort(v->begin(), v->end()); // 1 1 2 2 3 3 3 4 4 5 5 6 7 
				const auto last = std::unique(v->begin(), v->end());
				v->erase(last, v->end()); // vain samoja

				for (int k = 0; k < v->size(); k++)
				{
					for (int kk = k + 1; kk < v->size(); kk++)
					{
						circleCollision(v->at(k), v->at(kk));
						++count;
					}
				}
			}
		}
	}
	return count;
}

void AddToBucket(v2 pos, float w)
{
}

void insertObject(SpatialHash* hash, PhysicsBody* body, v2 point)
{
	v2 p = HashPoint((int)body->x, (int)body->y);
	hash->hashMap[p.y][p.x].push_back(body);
}

void InsertBox(SpatialHash* hash, PhysicsBody* body, v2 point)
{
}


struct PhysicsOut
{
	float x, y;
	float w, h;
	int textureId;
};


PhysicsBody physicsBodies[25000];
int currentCount = 0;
void addBody(float x, float y, float r, int id)
{
	physicsBodies[currentCount++] = PhysicsBody{ x, y, r, id };
}


// onko luoti Bodyn päällä
bool circleCollision(BulletBody* a, PhysicsBody* b)
{
	const float MIN_DISTANCE = a->r + b->r;

	glm::vec2 centerPosA = glm::vec2{ a->position.x, a->position.y } +glm::vec2(a->r);
	glm::vec2 centerPosB = glm::vec2{ b->x, b->y } +glm::vec2(b->r);
	glm::vec2 distVec = centerPosA - centerPosB;
	const float distance = glm::length(distVec);
	const float collisionDepth = MIN_DISTANCE - distance;

	return (collisionDepth > 0);
}

void circleCollision(PhysicsBody* a, PhysicsBody* b)
{
	const float MIN_DISTANCE = a->r * 2.0f;

	glm::vec2 centerPosA = glm::vec2{ a->x, a->y } +glm::vec2(a->r);
	glm::vec2 centerPosB = glm::vec2{ b->x, b->y } +glm::vec2(b->r);
	glm::vec2 distVec = centerPosA - centerPosB;

	const float distance = glm::length(distVec);
	const float collisionDepth = MIN_DISTANCE - distance;

	if (collisionDepth > 0)
	{
		if (distance == 0.f)
		{
			a->x += 30.f;
			a->y += 30.f;
			b->x -= 30.f;
			b->y -= 30.f;
			return;
		}

		const glm::vec2 collisionDepthVec = glm::normalize(distVec) * collisionDepth;

#if 0
		if (std::max(distVec.x, 0.0f) < std::max(distVec.y, 0.0f))
		{
			if (distVec.x < 0)
				b->x -= collisionDepthVec.x;
			else
				b->x += collisionDepthVec.x;
		}
		else
		{
			if (distVec.y < 0)
				b->y += collisionDepthVec.y;
			else
				b->y -= collisionDepthVec.y;
		}
		// b->y += collisionDepthVec.y;
#else
		const glm::vec2 aResolution = collisionDepthVec / 2.f; // +=
		a->x += aResolution.x;
		a->y += aResolution.y;

		const glm::vec2 bResolution = collisionDepthVec / 2.0f;  // -=
		b->x -= bResolution.x;
		b->y -= bResolution.y;

		// printf("%f\n", collisionDepthVec.x);
		// agent->_position -= collisionDepthVec / 2.0f;
#endif
	}
}

void removeBody(int id)
{
	for (int i = 0; i < currentCount; i++)
	{
		if (physicsBodies[i].owner == id)
		{
			// physicsBodies[i].
		}
	}
}

void updateCollision(PhysicsBody* bodies, int count)
{
	for (int i = 0; i < count; i++)
	{
		PhysicsBody* current = bodies + i;
		for (int j = i + 1; j < count; j++)
		{
			PhysicsBody* target = bodies + j;
			circleCollision(current, target);
		}
	}
}

void renderPhysicsBodies(PhysicsBody* bodies, int count)
{
	for (int i = 0; i < count; i++)
	{
		PhysicsBody* current = bodies + i;
		static UpiEngine::ColorRGBA8 white(255, 255, 255, 255);
		Debug::drawCircle(glm::vec2{ current->x, current->y }, white, current->r);
	}
}

//void circleCircleCollision(Entity &ball, Entity &pin) noexcept
//	{
//
//		if (!circleCollision(ball.getComponent<CCircle>(),
//			pin.getComponent<CCircle>())) return;
//
//		auto& ballP(ball.getComponent<CPhysics>());
//		auto& pinP(pin.getComponent<CPhysics>());
//		static const float ballVelocity{ 0.3f };
//
//		//ballP.m_velocity.y = -ballVelocity;
//		constexpr float BOOST_X{ 1.01f };
//		constexpr float BOOST_Y{ 1.01f };
//
//		pinP.m_velocity = ballP.m_velocity;
//
//		pinP.m_velocity.x *= BOOST_X;
//		pinP.m_velocity.y *= BOOST_Y;
//		
//		const sf::Vector2f friction(0.99f, 0.99f);
//		ballP.m_velocity.x *= friction.x;
//		ballP.m_velocity.y *= friction.y;
//
//		constexpr float TURN_RATE{ 0.005f };
//		if (ballP.x() < pinP.x())	// vasemmalle palloa
//		{
//			ballP.m_velocity.x -=  TURN_RATE;
//		}
//		else
//		{
//			ballP.m_velocity.x +=  TURN_RATE;
//		}
//		pin.getComponent<CKillComponent>().setKillTimer(1000.f);
//	}

// tama jarjestys:    1, 2, 3, 4, 5, 6
//               :    4, 8, 4, 2, 8, 8       <-sorttaa

//                    2  5  6  3  1  4
//                    8  8  8  4  4  2


//  entity[0]  =  entity[2]
//  entity[1]  =  entity[5] 

struct IndexValue
{
	int index, type;
};

static Entity* temp = new Entity;
void SortAllEntitys(Entity* entities, int count)
{
	//temp = new Entity;
	IndexValue* currentOrder = new IndexValue[count];

	for (int i = 0; i < count; i++)
	{
		currentOrder[i] = { i, entities[i].type };
	}

	std::sort(currentOrder, currentOrder + count, [](IndexValue& a, IndexValue& b) {
		return a.type < b.type;
	});

	for (int i = 0; i < count; i++)
	{
		memcpy(&temp, (entities + i), sizeof(Entity));
		memcpy(entities + i, entities + currentOrder[i].index, sizeof(Entity));
		memcpy(entities + currentOrder[i].index, &temp, sizeof(Entity));
	}

	delete[] currentOrder;
}


void SortAll(Entity* entities, int count) // 
{
	//std::sort(entities, entities + count, [](Entity& first, Entity& second)
	//{
	//	return first.type < second.type;
	//});
}


game_state* luasgamestate = nullptr;
EXPORT __declspec(dllexport) void* getGameState()
{
	return luasgamestate;
}

using namespace std;
StackAllocator g_singleFrameAllocator;

ImageData::ImageData(const char* filename)
{
	surface = IMG_Load(filename);
	if (!surface)
		printf("IMG_Load: %s\n", IMG_GetError());
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



void RemoveEntity(const int index, game_state* state)
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
//   * 
//   * Countries -> data structure for owned provinces etc...
//   * Troops/Units -> freely moving units on the screen different (sides color)
//   * defensive structures 
//   * province logic
//   * factory/mill outputtin units/cash
//   * 

//  * AI:
//  * super simple just sittings there maybe building buildings?
//  * ----zombie mode----
//  * zombie ai just walking towards closest one
//  * simple neutral countries trying to stay alive

//  * boats
//  * technology mode
//  * diplomacy

// Engine:
//   * Hotloading variable tweak -> (from google sheets maybe?)
//   * Configs files
//   * Better solutions for -> . -> . 
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

#define IM_COL32_R_SHIFT    0
#define IM_COL32_G_SHIFT    8
#define IM_COL32_B_SHIFT    16
#define IM_COL32_A_SHIFT    24
#define IM_COL32_A_MASK     0xFF000000
struct v4 { float x, y, z, w; };
v4 ColorConvertU32ToFloat4(Uint32 in)
{
	float s = 1.0f / 255.0f;
	return{
		((in >> IM_COL32_R_SHIFT) & 0xFF) * s,
		((in >> IM_COL32_G_SHIFT) & 0xFF) * s,
		((in >> IM_COL32_B_SHIFT) & 0xFF) * s,
		((in >> IM_COL32_A_SHIFT) & 0xFF) * s };
}

#include <sstream>
//  #include <>

// Graph<int, int> map(6);
//struct MapNode
//{
//	int   id;
//	float x, y; // for rendering and debugging 
//};

#define MAX_PROVINCES 128
Uint32 idToColor[MAX_PROVINCES];
v2 positions[MAX_PROVINCES];
std::map<Uint32, int> colorToId;
int nodeCount = 0;
Graph<MapNode, int> nodes(64);

void SaveNodes()
{
	FILE* file = fopen("test2.txt", "w");
	const int BufferSize = 256;
	char buffer[BufferSize];

	// ;id;x;y;r;g;b;a;n;n;n;n;n;n;n;n;n;n;
	for (int i = 0; i < nodeCount; i++)
	{
		auto color = ColorConvertU32ToFloat4(idToColor[i]);
		v2 pos = positions[i];
		int count = sprintf(buffer, "%i;%i;%i;%i;%i;%i;%i;",
			i, pos.x, pos.y, (int)(color.w * 255), (int)(color.z * 255), (int)(color.y * 255), (int)(color.x * 255));


		for (auto iter = nodes.nodes[i]->archlist.begin(); iter != nodes.nodes[i]->archlist.end() &&
			count + sizeof(int) < BufferSize; iter++)
		{
			count += sprintf(buffer + count, "%i;", iter->node->data.id);
		}
		count += sprintf(buffer + count, "\n");

		fwrite(buffer, sizeof(char), count, file);
	}
	fclose(file);
}

Uint32 createRGBA(int r, int g, int b, int a)
{
	return ((r & 0xff) << 24) + ((g & 0xff) << 16) + ((b & 0xff) << 8)
		+ (a & 0xff);
}

#include "graph.h"
#include <iostream>
#include <fstream>

inline int GetNextInt(std::istringstream& stream, std::string& string, const char* title)
{
	std::getline(stream, string, ';');
	std::cout << title << string << ", ";
	return atoi(string.c_str());
}

void LoadNodes()
{
	const int maxNeighbours = 8;
	int* arcs = (int*)malloc(sizeof(int) * 64 * 8);
	memset(arcs, -1, sizeof(int) * 64 * 8);

	std::ifstream stream("test2.txt");
	std::string line;
	int lastSaved = 0;

	while (std::getline(stream, line))
	{
		std::istringstream s(line);

		if (line.at(0) == ';') // rivi on kommentti ;
			continue;
		nodeCount++;

		std::string field;

		int id = GetNextInt(s, field, "id: ");
		int x = GetNextInt(s, field, "x: ");
		int y = GetNextInt(s, field, "y: ");

		int r = GetNextInt(s, field, "r: ");
		int g = GetNextInt(s, field, "g: ");
		int b = GetNextInt(s, field, "b: ");
		int a = GetNextInt(s, field, "a: ");

		// int break2 = lastSaved * 2;
		positions[id] = { x, y };
		Uint32 color = createRGBA(r, g, b, a);

		colorToId[color] = id;
		idToColor[id] = color;

		MapNode node{ id, (float)x, (float)y };
		nodes.AddNode(node, id);

		std::cout << "Neighbours: ";
		int i = 0;
		while (std::getline(s, field, ';') && i < maxNeighbours)
		{
			std::cout << field << ", ";
			int neighboursId = atoi(field.c_str());
			*(arcs + id * maxNeighbours + i) = neighboursId;
			i++;
		}
		std::cout << "\n";
		lastSaved = id;
	}

	printf("Adding Arcs!\n");
	// set the arcs!
	for (int id = 0; id < lastSaved + 1; id++)
	{
		for (int n = 0; n < maxNeighbours && (*(arcs + id * maxNeighbours + n)) != -1; n++)
		{
			int neigbourdId = *(arcs + id * maxNeighbours + n);
			nodes.AddArc(id, neigbourdId, 1); // Note: Are weights ever needed!
			printf("added arc from %i to %i \n", id, neigbourdId);
		}
	}

	free(arcs);
}

#include <map>
std::vector<int> BreadthFirst(int startID, Graph<MapNode, int>* graph, int goalId)
{
	GraphNode<MapNode, int>* start = graph->nodes[startID];

	std::queue<GraphNode<MapNode, int>*> queue;
	queue.push(start);
	start->marked = true;

	std::map<GraphNode<MapNode, int>*, GraphNode<MapNode, int>*> came_from;
	came_from[start] = 0; 

		while (queue.size() != 0)
		{
			GraphNode<MapNode, int>* current = queue.front();

			if (current->data.id == goalId)
				break;

			auto itr = current->archlist.begin(); // neighbours !
			for (itr; itr != current->archlist.end(); itr++)
			{
				if (itr->node->marked == false)
				{
					itr->node->marked = true;
					queue.push(itr->node);
					came_from[itr->node] = current;
				}
			}
			queue.pop();
		}

	// make path
	GraphNode<MapNode, int>* current = graph->nodes[goalId];
	std::vector<GraphNode<MapNode, int>*> path;


	std::vector<int> returnPath;
	returnPath.reserve(16);

	path.push_back(current);
	returnPath.push_back(current->data.id);

	while (current != start)
	{
		current = came_from[current];

		if (current == 0)
		{
			printf("could't find path between %i and %i\n", startID, goalId);
			ASSERT(false);
			return returnPath;
		}
		else
		{
			path.push_back(current);
			returnPath.push_back(current->data.id);
		}
	}
	printf("the path between %i and %i is\n", startID, goalId);
	for (int i = 0; i < path.size(); i++)
	{
		auto node = path[i];
		printf("%i, ", node->data.id);
	}
	printf("\n");
	graph->ClearMarks();

	return returnPath;

	// Python: reverse:

	// path = [current]
	// while current != start:
	//		current = came_from[current]
	//		path.append(current)

	//	path.append(start) 
	//	path.reverse()      
}


void newNode(int index, int id, float x, float y)
{
	MapNode newNode{ id, x, y };
	nodes.AddNode(newNode, index);
}

// float x = e->x - bullet->startX;
// float y = e->y - bullet->startY;
// if (abs(x * x + y * y) > bullet->rangeSquared)
// {
//		max range reached!
//		e->alive = false;
//		printf("olen donezo\n");
// }


int simulateBullets(BulletBody* bodies, vec2f* bulletAcceceration, BulletStart* start, int count)
{
	for (int i = count - 1; i > -1; i--)
	{
		bodies[i].position += bulletAcceceration[i]; // TODO: DELTA JOUJOUJOU!

		float x = bodies[i].position.x - start[i].position.x;
		float y = bodies[i].position.y - start[i].position.y;

		if (abs(x * x + y * y) > start[i].rangeSqrt)
		{
			// luoti kuoli! poista kaikki kerralla?
			bodies[i]				= bodies[count - 1];
			start[i]				= start[count - 1];
			bulletAcceceration[i]   = bulletAcceceration[count - 1];

			printf("kuoli");
			--count;
		}
	}

	return count;
}

int collideBullets(SpatialHash* hash, BulletBody* bulletBodies, int count)
{
	for (int i = count - 1; i > -1; i--)
	{
		BulletBody* body = bulletBodies + i;

		auto point = HashPoint(body->position.x, body->position.y);
		auto* bodies = &hash->hashMap[point.y][point.x];

		for (int j = 0; j < bodies->size(); j++)
		{
			if (circleCollision(body, bodies->at(j)))
			{
				// bulletBodies->at(i)->owner <- ammu tota jätkää

				bulletBodies[i] = bulletBodies[count - i];
				--count;
				break;
			}
		}
	}
	return count;
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
		memory_arena* arena = &gameState->arena;

		luasgamestate = gameState; // game_state -> lua

		core->memory->isInitialized = true;

		initSpatial(&hash4r);

		gameState->MapNodes = &nodes;

		L = core->script.L;
		Debug::_Debugger = core->debugger; // hööh :-(
		UpiEngine::ResourceManager::SetContext(core->ctx.textureCacheCtx);


		printf("##################################################\n");
		LoadNodes();
		printf("##################################################\n");


		gameState->provinceData.maxProvinces = MAX_PROVINCES;
		gameState->provinceData.currentCount = &nodeCount;
		gameState->provinceData.positions = positions;
		gameState->provinceData.colorToId = &colorToId;
		gameState->provinceData.idToColor = idToColor;

		gameState->selectedEntitys = ((Entity**)PushArray2(arena, 1000, 4));
		gameState->maxSelected = 1000;

		// TODO: defaults some neat fileloading facilities would be cool
		gameState->cameraSpeed = 50.0f;

		gameState->entities[0].type = Entity_ninja;
		gameState->entities[1].type = Entity_npc;

		int j = 0;
		for (int i = 0; i < (int)ArrayCount(gameState->entities); i++)
		{
			gameState->entities[i].guid = i;
			j++;
			gameState->entities[i].unit.targetX = -1;
			gameState->entities[i].unit.targetY = -1;
			gameState->entities[i].alive = true;
		}

		gameState->entities[2].type = Entity_unit;
		gameState->entities[2].x = 300.f;
		gameState->entities[2].y = 300.f;
		gameState->entities[2].unit.side = 0xFFFF0000; // ABGR
		gameState->entities[2].unit.attackRange = 250.f;
		// gameState->entities[2].unit.mainAttackCD = 

		gameState->entities[3].type = Entity_player;
		gameState->entities[3].player.cash = 100;
		gameState->entities[3].player.side = 0xFF00FF00; // green
		gameState->player = &gameState->entities[3];
		gameState->currentEntityCount = 4;


		gameState->getAllProvinceNeighbours = getAllProvinceNeighbours;
		gameState->newNode = newNode;

		// init some npc's
		Entity* e = GetFirstAvaibleEntity(gameState);
		e->type = Entity_script;
		auto script = GET_ENTITY(e, script);
		sprintf(script->message, "%s", "Morjesta"); // Luassa olisi kiva maaritella jutut
		script->hitbox = { 0, 0, 40, 80 };

		// world map 
		SDL_Surface* surface = core->resources.LoadSurface("europedata.png"); // clickaamiseen
		gameState->worldmap.provinces.surface = surface;

		surface = core->resources.LoadSurface("europedata.png");
		gameState->worldmap.visual.surface = surface;
		gameState->worldmap.temptextureid = core->resources.SurfaceToGlTexture(surface);

		gameState->worldmap.dimensions = glm::vec4{ 0.f, 0.f, surface->w * mapSizeMultiplier, surface->h * mapSizeMultiplier };



		addBody(20, 20, 10, 0);
		addBody(40, 40, 10, 1);




		if (!Debug::restartLog())
		{
			printf("log restart failed: %s, %d", __FILE__, __LINE__);
		}

		nodes.ClearMarks();
		//	BreadthFirst(0, &nodes, 7);
	}


	/*************************************************/
	// ENTITY UPDATE
	for (int i = gameState->currentEntityCount - 1; i > -1; i--)
	{
		f(&gameState->entities[i], core);
		if (!gameState->entities[i].alive)
		{
			// deletefunc
			memcpy(&gameState->entities[i], &gameState->entities[gameState->currentEntityCount - 1],
				sizeof(Entity));
			--gameState->currentEntityCount;
		}
	}



	// TODO: Nuke this init phase
	static bool init = false;
	if (!init) // || core->input->isKeyPressed(SDL_SCANCODE_SPACE))
	{
		Random::init();

		init = true;
	}

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
		//lua_State* L = core->script.L;
		//lua_getglobal(L, "LoadTileMap");
		//lua_pushnumber(L, 100);

		//if (lua_pcall(L, 1, 0, 0) != 0)
		//{
		//	Debug::logError("what the fuck!");
		//	Debug::logError(lua_tostring(L, -1));
		//	fprintf(stderr, "%s\n", lua_tostring(L, -1));
		//	debugBreak();
		//}
		//lua_pop(L, 1);
	}

	static bool initted = false;
	static GLuint tid;
	if (!initted)
	{
		initted = true;
		// showToPlayer
		// tid = core->resources.SurfaceToGlTexture(gameState->worldmap.visual.surface);
		// FreeTexture(&tid);
	}

	// MOUSE debug code
	WorldMap* map = &gameState->worldmap;

	glm::vec2 mxy = gameState->worldmap.GetMouse(&input->mouse);
	float mx = mxy.x;
	float my = mxy.y;
	Debug::drawBox(mx, my, 3.f, 3.f);

	if (input->isKeyDown(SDL_SCANCODE_4))
	{
		Uint32 color = gameState->worldmap.provinces.GetPixel(mx, my); // real size 
		auto iter = colorToId.find(color);

		if (iter != colorToId.end())
			printf("id: %x\n ", (iter->second));
		else
			printf("%x not found\n", color);
	}



	if (input->isKeyDown(SDL_SCANCODE_6))
	{
		WorldMap* map = &gameState->worldmap;

		Uint32 targetcolor = gameState->worldmap.provinces.GetPixel((int)mx, (int)my);

		SDL_PixelFormat *fmt = gameState->worldmap.provinces.surface->format;
		Uint8 alpha = getAlpha(targetcolor, fmt);


		if (targetcolor != 0xFF000000 && alpha != 0)
		{
			FloodFillImage(&map->provinces, &map->visual, mx, my, targetcolor, Uint32(map->editor.editorColor));

			// core->resources.FreeTexture(&tid);
		//	gameState->worldmap.temptextureid = core->resources.SurfaceToGlTexture(gameState->worldmap.visual.surface);

			core->resources.FreeTexture(&map->temptextureid);
			core->resources.SurfaceToGlTexture(map->visual.surface);
		}
	}


	if (input->isMouseDown(3)) // atm imgui toolssien nappi TODO: move
	{
		WorldMap* map = &gameState->worldmap;
		Uint32 color = gameState->worldmap.provinces.GetPixel(mx, my); // real size 
		map->editor.editorColor = color;

		// printf("%x\n", color);
		// printf("%x\n", idToColor[0]);
		// printf("%i ", idToColor[0] == color);

		// WorldMap* map = &gameState->worldmap;
		map->editor.inputX = input->mouse.x;
		map->editor.inputY = input->mouse.y;

	}

	if (input->isKeyPressed(SDL_SCANCODE_9) || gameState->dirtyFlag)
	{
		SaveNodes();
		// new nodes
		// nodes.nodes[gameState->provinceEditor.selectedProvinceId]->;
		gameState->dirtyFlag = false; //FUUUUUUUUUUUUUCKKCKUFFUUUUUUUUUUUUUUUUUCK FUUCK FUCK
	}

	if (input->isKeyPressed(SDL_SCANCODE_H))
	{
		SortAllEntitys(gameState->entities, gameState->currentEntityCount);
	}
}

/* Get Red component */
//temp = pixel & fmt->Rmask;  /* Isolate red component */
//temp = temp >> fmt->Rshift; /* Shift it down to 8-bit */
//temp = temp << fmt->Rloss;  /* Expand to a full 8-bit number */
//red = (Uint8)temp;
//
///* Get Green component */
//temp = pixel & fmt->Gmask;  /* Isolate green component */
//temp = temp >> fmt->Gshift; /* Shift it down to 8-bit */
//temp = temp << fmt->Gloss;  /* Expand to a full 8-bit number */
//green = (Uint8)temp;
//
///* Get Blue component */
//temp = pixel & fmt->Bmask;  /* Isolate blue component */
//temp = temp >> fmt->Bshift; /* Shift it down to 8-bit */
//temp = temp << fmt->Bloss;  /* Expand to a full 8-bit number */
//blue = (Uint8)temp;
//
///* Get Alpha component */
//temp = pixel & fmt->Amask;  /* Isolate alpha component */
//temp = temp >> fmt->Ashift; /* Shift it down to 8-bit */
//temp = temp << fmt->Aloss;  /* Expand to a full 8-bit number */
//alpha = (Uint8)temp;





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

void VisualizePath(std::vector<int>* path, game_state* state)
{
	if (path->size() <= 0)
	{
		printf("no path");
		return;
	}

	v2 lastPos = state->provinceData.positions[path->at(path->size() - 1)];
	for (int i = path->size() - 1; i > -1; i--)
	{
		int provinceID = path->at(i);
		auto v2 = state->provinceData.positions[provinceID];

		Debug::drawLine({ (float)lastPos.x, (float)lastPos.y }, { (float)v2.x, (float)v2.y });
		Debug::drawBox({ v2.x, v2.y, 20, 20 });
		lastPos = v2;
	}

}

std::vector<int> getAllProvinceNeighbours(int id)
{
	std::vector<int> returnValue;

	if (nodes.nodes[id] != 0)
	{
		for (auto iter = nodes.nodes[id]->archlist.begin(); iter != nodes.nodes[id]->archlist.end(); ++iter)
		{
			returnValue.push_back(iter->node->data.id);
		}
	}

	return returnValue;
}


// return {};

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

// EXPORT __declspec(dllexport) typedef struct
// {
	// float x;
	// float y;
// } v2;

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
		// al_/draw_bitmap(bmp, data[i].x, data[i].y, 0);
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

EXPORT __declspec(dllexport) void Drawer(float x, float y) //LUA_E_F
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


struct exportMe { //LUA_E_S
	int a;
	float b;
	const char* name;
};             //LUA_E_SE

EXPORT __declspec(dllexport) void testFuncy(int a, int b) //LUA_E_F
{
	printf("hello world (%i,%i)\n", a, b);
}


void dumpStruct(Uint32 memberCount, member_definition* memberData, void* structPtr, int intendLevel = 0)
{
	// char buffer[512];
	for (u32 memberIndex = 0; memberIndex < memberCount; ++memberIndex)
	{
		char TextBufferBase[256];
		char *textBuffer = TextBufferBase;
		for (int intend = 0; intend < intendLevel; ++intend)
		{
			*textBuffer++ = ' ';
			*textBuffer++ = ' ';
			*textBuffer++ = ' ';
			*textBuffer++ = ' ';
		}
		textBuffer[0] = 0;

		member_definition* member = memberData + memberIndex;
		void *memberPtr = (((Uint8 *)structPtr) + member->offset);

		if (member->flags & MetaMemberFlag_IsPointer)
		{
			memberPtr = *(void **)memberPtr;
		}

		if (memberPtr)
		{


			int textBufferLeft = TextBufferBase + sizeof(TextBufferBase) - textBuffer;
			switch (member->type)
			{
			case MetaType_bool32:
				_snprintf_s(textBuffer, textBufferLeft, textBufferLeft, "%s: %b ", member->name, *(Uint32 *)memberPtr);
				break;
			case MetaType_int:
				_snprintf_s(textBuffer, textBufferLeft, textBufferLeft, "%s: %i ", member->name, *(int *)memberPtr);
				break;
			case MetaType_float:
				_snprintf_s(textBuffer, textBufferLeft, textBufferLeft, "%s: %f ", member->name, *(float *)memberPtr);
				break;
			case MetaType_uint32:
				_snprintf_s(textBuffer, textBufferLeft, textBufferLeft, "%s: %i ", member->name, *(Uint32 *)memberPtr);
				break;

				META_HANDLE_TYPE_DUMD(memberPtr, intendLevel + 1)

					/*case MetaType_v3:

						DEbugdumpt suct ( Arraycount membersof v3,  membesfof ve3,
							memberptr
					} break;*/
			}
			printf("%s\n", textBuffer);
		}
	}
}


//void startTiming(int line)
//{
//}
////    int a = __LINE__;
//
//void endTiming()
//{
//}

//auto timePoint1(std::chrono::high_resolution_clock::now());
//auto delta_time = std::chrono::high_resolution_clock::now() - currentTime;
//currentTime = std::chrono::high_resolution_clock::now();
//simulationTime += std::chrono::duration_cast<std::chrono::nanoseconds>(delta_time);
//
//auto timePoint2(std::chrono::high_resolution_clock::now());
//auto elapsedTime(timePoint2 - timePoint1);
//ft = { std::chrono::duration_cast<std::chrono::duration<float, std::milli>>(elapsedTime).count() };
//lastFT = ft;
//ftSeconds = (ft / 1000.f);


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

	// core->spriteBatch->draw(glm::vec4{ 200.f, 0.f, 590.f , 480.f  }, glm::vec4{ 0.f, 0.f, 1.0f, 1.0f }, gameState->worldmap.temptextureid, 1.0f);



	gameState->worldmap.Draw(core->spriteBatch);

	//  DrawAllEntitys(core);

	// render all entitys

	for (int i = 0; i < gameState->currentEntityCount; i++)
	{
		Entity* e = &gameState->entities[i];
		r(e, core);
	}

	if (gameState->pathfindingUi.drawPath)
	{
		auto* ui = &gameState->pathfindingUi;
		auto path = BreadthFirst(ui->startId, &nodes, ui->endId);
		VisualizePath(&path, gameState);
	}

	// fysiikka	       --::testi::--



	clearSpatial(&hash4r);


	currentCount = 0;
	for (int i = 0; i < gameState->currentEntityCount; i++)
	{
		Entity* e = &gameState->entities[i];
		if (e->type == Entity_unit)
		{
			*(physicsBodies + currentCount) = { e->x, e->y, 15.f, (int)e->guid };
			AddBodyToGrid(physicsBodies + currentCount, &hash4r);
			++currentCount;
		}
	}


	// Physics step:

	// updateCollision(physicsBodies, currentCount);
	// renderPhysicsBodies(physicsBodies, currentCount);

	// int cols = CheckCollisions(&hash4r);
	// printf("collisions: %i  entitys: %i \n", cols, currentCount);

	// updateCollision()
	//	Entity* e =gameState->entities[i];

	for (int i = 0; i < currentCount; i++)
	{
		PhysicsBody* body = physicsBodies + i;
		Entity* e = &gameState->entities[body->owner];
		e->x = body->x;
		e->y = body->y;
	}

	// Sta/r
	gameState->bulletCount = simulateBullets(gameState->bulletBodies, gameState->BulletAccelerations,
		gameState->bulletStart, gameState->bulletCount);

	gameState->bulletCount = collideBullets(&hash4r, gameState->bulletBodies, gameState->bulletCount);

	for(int i = 0; i < gameState->bulletCount; i++)
	{
		const auto& pos = gameState->bulletBodies[i].position;
		core->spriteBatch->draw(glm::vec4{ pos.x, pos.y, 15.f, 15.f }, glm::vec4{ 0.f, 0.f, 1.f, 1.f },
			3, 1.0f); // depth !
	}


	static bool diebugDraw = false;
	if (diebugDraw)
	{
		// drawm
		debugDraw(&hash4r);
	}

	// dumpStruct(ArrayCount(membersOf_test2), membersOf_test2, &test2);

	// for (int i = 0; i < gameState->selectedCount; i++) {
		// Entity* e = gameState->selectedEntitys[i];
		// float r = e->unit.attackRange;
		// static UpiEngine::ColorRGBA8 white(255, 255, 255, 255);
		// Debug::drawCircle({ e->x, e->y }, white, r);
	// }

}


// TODO: Fontit resource managerille

// LUOTEJA
