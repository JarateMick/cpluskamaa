#include "Entity.h" 
#include "core.h"
#include "game.h"
#include "random.h"
#include <ResourceManager.h>

// std::vector<int> BreadthFirst(int startID, Graph<MapNode, int>* graph, int goalId)

//std::vector<int> findPath(int startId, int goalId, game_state* gameState)
//{
//	return BreadthFirst(startId, gameState->MapNodes, goalId);
//}

const float UNIT_SPEED = 1.0f;   // ~ archer speed
const int   UNIT_BASE_HP = 100;
const float BULLET_BASE_SIZE = 5.f;    //   sqrt(18) = 4.2
constexpr float SHOTGUN_SPREAD = 15.f;
constexpr float SHOTGUN_SPREAD_HALF = 15.f / 2.f;
constexpr float MELEE_RANGE = 60.f;
constexpr float RANGED_RANGE = 250.f;

//  one-to-many      many-to-one

static inline Uint32 BodyToSide(PhysicsBody* body, game_state* gameState)
{
	return gameState->allSides[body->owner];
}

static inline Entity* getEntity(int id, game_state* gameState)
{
	return &gameState->entities[id];
}

Entity* createUnit(float x, float y, EngineCore* core)
{
	GetGameState(core);
	PhysicsBody* physicsBodies = gameState->bodies;

	Entity *ee = newEntity(x + Random::floatInRange(-25.f, 25.f), y + Random::floatInRange(-25.f, 25.f), Entity_unit, gameState);
	(physicsBodies + ee->guid)->r = 15.f;
	(physicsBodies + ee->guid)->owner = ee->guid;
	// printf("%f, %f", ee->x, ee->y);
	ee->unit.attackRange = 250.f;
	ee->unit.targetX = -1;
	ee->unit.targetY = -1;
	ee->unit.originalTargetX = -1;
	ee->unit.originalTargetY = -1;
	ee->unit.hp = UNIT_BASE_HP;
	ee->unit.movementSpeed = UNIT_SPEED;

	Uint32 side = gameState->worldmap.editor.editorColor;
	if (side == 0)
	{
		ee->unit.side = 0xFFFF00FF;
		gameState->allSides[ee->guid] = 0xFFFF00FF; // tarkka kenen guid fuck
		setEntityColor(0xFFFF00FF, gameState, ee->guid);
	}
	else
	{
		ee->unit.side = side;
		gameState->allSides[ee->guid] = side;
		setEntityColor(side, gameState, ee->guid);
	}

	// olisi varmaan kivempi etta kaikki ottas guiding naaa 
	AddBodyToGrid(physicsBodies + ee->guid, &hash4r, gridPositions + ee->guid);
	InitAnimation(gameState, ee->guid);

	UnitStructure* unitdata = &gameState->unitStructure;
	unitdata->attackTargets[ee->guid] = nullptr;
	unitdata->attackTypes[ee->guid] = attack_ranged;

	return ee;
}

void setAttackType(int id, game_state* gameState, attack_type type)
{
	gameState->unitStructure.attackTypes[id] = type;
}

Entity* createUnit(float x, float y, game_state* gameState, Uint32 side, attack_type type, float attackRange)
{

	// float x = (body + e->guid)->x;
	// float y = (body + e->guid)->y;
	Entity *ee = newEntity(x + Random::floatInRange(-25.f, 25.f), y + Random::floatInRange(-25.f, 25.f), Entity_unit, gameState);
	PhysicsBody* body = &gameState->bodies[ee->guid];
	body->r = 15.f;
	body->owner = ee->guid;
	ee->unit.attackRange = attackRange;
	ee->unit.targetX = -1;
	ee->unit.targetY = -1;
	ee->unit.originalTargetX = -1;
	ee->unit.originalTargetY = -1;
	ee->unit.side = side;
	ee->unit.hp = UNIT_BASE_HP;
	// ee->unit.attackType = type;
	setAttackType(ee->guid, gameState, type);
	ee->unit.movementSpeed = UNIT_SPEED;

	gameState->allSides[ee->guid] = side; // tarkka kenen guid fuck
	AddBodyToGrid((body), gameState->spatialGrid, gridPositions + ee->guid);

	// setup color
	Uint32 ucolor = ee->unit.side;
	setEntityColor(ucolor, gameState, ee->guid);

	InitAnimation(gameState, ee->guid);

	UnitStructure* unitdata = &gameState->unitStructure;
	unitdata->attackTargets[ee->guid] = nullptr;

	// Entity* attackTargets[MAX_ENTITY_COUNT];
	// attack_type attackTypes[MAX_ENTITY_COUNT];
	return ee;
}

static void setPathfindTarget(game_state* gameState, std::vector<int>& path, InputManager* input, int i, int x, Entity* e)
{
	int targetId;
	if (path.size() > 2)
	{
		targetId = path[path.size() - 2]; // eka on maali
		auto v2 = gameState->provinceData.positions[targetId];
		gameState->selectedEntitys[i]->unit.targetX = v2.x;
		gameState->selectedEntitys[i]->unit.targetY = v2.y;

		gameState->selectedEntitys[i]->unit.originalTargetX = input->mouse.x;
		gameState->selectedEntitys[i]->unit.originalTargetY = input->mouse.y;

		printf("matka\n"); // seuraa reittiä
	}
	else  // mee viereiseen // @cleanup
	{
		targetId = path[0];
		auto v2 = gameState->provinceData.positions[targetId];
		gameState->selectedEntitys[i]->unit.targetX = v2.x;
		gameState->selectedEntitys[i]->unit.targetY = v2.y;

		gameState->selectedEntitys[i]->unit.targetX = input->mouse.x;

		gameState->selectedEntitys[i]->unit.targetY = input->mouse.y;
		gameState->selectedEntitys[i]->unit.originalTargetX = input->mouse.x;
		gameState->selectedEntitys[i]->unit.originalTargetY = input->mouse.y;

		int guid = gameState->selectedEntitys[i]->guid;

		Anim_enum animation = (input->mouse.x < x) ? Anim_Archer_Run_Left : Anim_Archer_Run_Right;
		SetAnimation(gameState, guid, animation);
	}

	e->unit.nextProv = IdToColor(targetId, gameState);

	printf("%i next province \n", targetId);
}

// keep track of current amount of zombies
// targetit ja tyypit tableen!
void UpdateAi(ZombieAi* ai, game_state* gameState)
{
	ai->timeToNextSpawn -= (1 / 60.f);
	if (ai->timeToNextSpawn < 0.f)
	{
		ai->timeToNextSpawn = Zombie::spawnTimer;

		printf("Spanwn\n");

		v2 startPos = gameState->provinceData.positions[ai->spawnProvinceId];

		for (int i = 0; i < ai->spawnCount; i++)
		{
			startPos.x++;

			if (i % 25 == 0)
			{
				startPos.y++;
				startPos.x -= 25.f;
			}

			Entity* e = createUnit(startPos.x, startPos.y, gameState, Zombie::zombieSide, attack_zombie, MELEE_RANGE); // TODO: meleet toimii viela huonosti mutta jonain paivana!!!!!!!!!!!!!!!!!!!!!!!!

			e->unit.lookingForTarget = true;
			// zombeille autoattack paalle niin voivat hyokata
		}
	}
	// laita uusille zombeille targetiksi pelaaja tai jotain
	// kahtele unit listasta, jokin yksikkö ja lähetä sen sijaintiin zombit  jos ei ole vene!

	// zombie veneet ?

	Entity* target = 0;
	for (int i = 0; i < gameState->currentEntityCount; i++)
	{
		if (gameState->unitStructure.attackTypes[i] != attack_zombie)
		{
			target = &gameState->entities[i];
			break;
		}
	}

	if (target)
	{
		for (int i = 0; i < gameState->currentEntityCount; i++)
		{
			if (gameState->unitStructure.attackTypes[i] == attack_zombie && !gameState->unitStructure.attackTargets[i])
			{
				gameState->unitStructure.attackTargets[i] = target;
			}
		}
	}
}




UpiEngine::ColorRGBA8 Uin32ToColor(Uint32 color)
{
	GLubyte a = color >> 24 & 255;
	GLubyte r = color >> 16 & 255;
	GLubyte g = color >> 8 & 255;
	GLubyte b = color >> 0 & 255;
	return UpiEngine::ColorRGBA8(b, g, r, a); // uint32 on muodossa ABGR
}

void setEntityColor(Uint32 color, game_state* gameState, int guid)
{
	auto colr = Uin32ToColor(color);
	gameState->entityColors[guid] = colr; // drawing color
}

void dealDamage(Entity* target, int damage)
{
	target->unit.hp -= damage;
	if (target->unit.hp < 0)
		target->alive = false;
}

inline void SetTarget(Entity* unit, std::vector<int>& path, int id, ProvinceData* prov)
{
	int targetId = path[id]; // eka on maali
	auto v2 = prov->positions[targetId];
	unit->unit.targetX = v2.x;
	unit->unit.targetY = v2.y;
}

static bool SetNextPathNode(Entity* e, game_state* gameState)
{
	std::vector<int>& path = e->unit.path;
	if (path.size() > 2)
	{
		SetTarget(e, path, path.size() - 2, &gameState->provinceData);
		return true;
	}
	return false;
}

bool FollowPath(Entity* entityUnit, game_state* gameState, Uint32 currentSide)
{
	std::vector<int>& path = entityUnit->unit.path;
	if (path.size() > 2 && currentSide == entityUnit->unit.nextProv)
	{
		SetTarget(entityUnit, path, path.size() - 2, &gameState->provinceData);
		entityUnit->unit.nextProv = IdToColor(path.size() - 2, gameState);

		return true;
	}
	return false;
	// mee viereiseen

	// int targetId = path[path.size() - 1];
	// auto v2 = gameState->provinceData.positions[targetId];
	// gameState->selectedEntitys[targetId]->unit.targetX = v2.x;
	// gameState->selectedEntitys[targetId]->unit.targetY = v2.y;

	// gameState->selectedEntitys[i]->unit.targetX = path[path.size() - 1];
	// gameState->selectedEntitys[i]->unit.targetY = path[path.size() - =]
	// printf("lyhy\n");
}


void AddBullet(game_state* gameState, const glm::vec2 direction, float x, float y, Uint32 side, float range)
{
	BulletBody body;
	body.position.x = x; // e->x
	body.position.y = y; // e->y
	body.r = BULLET_BASE_SIZE;
	body.side = side;

	BulletStart start;
	start.position = body.position;
	start.rangeSqrt = range * range;

	gameState->bulletBodies[gameState->bulletCount] = body;
	gameState->BulletAccelerations[gameState->bulletCount] = direction;
	gameState->bulletStart[gameState->bulletCount] = start;
	++gameState->bulletCount;
}

void setArcherShootAnimation(game_state* gameState, glm::vec2 direction, PhysicsBody* body, Entity* playerControlled)
{
	Anim_enum animation = (body->x > body->x + direction.x) ? Anim_Archer_Shooting_Left : Anim_Archer_Shooting_Rigth;
	SetAnimation(gameState, playerControlled->guid, animation);
}

constexpr float playerControlledSpeed = 1.f;
void controlUnit(Entity* playerControlled, EngineCore* core, PhysicsBody* body)
{
	GetGameState(core);
	DefineInput(core);

	glm::vec2 dirVector = {};
	if (input->isKeyDown(SDL_SCANCODE_W))
	{
		dirVector.y = 1.f;
	}
	else if (input->isKeyDown(SDL_SCANCODE_S))
	{
		dirVector.y = -1.f;
	}

	if (input->isKeyDown(SDL_SCANCODE_A))
	{
		dirVector.x = -1.f;
	}
	else if (input->isKeyDown(SDL_SCANCODE_D))
	{
		dirVector.x = 1.f;
	}

	if (input->isKeyPressed(SDL_SCANCODE_R))
	{
		for (int i = -10; i < 10; i++)
		{
			for (int j = -10; j < 10; j++)
			{
				glm::vec2 direction{ i, j };
				direction = glm::normalize(direction);
				AddBullet(gameState, direction, body->x, body->y,
					gameState->allSides[playerControlled->guid], RANGED_RANGE);
			}
		}
	}

	if (input->isKeyPressed(SDL_SCANCODE_F))
	{
		float randomOffsetY = 0.f;
		float randomOffsetX = 0.f;

		glm::vec2 direction{};
		for (int i = 0; i < 10; i++)
		{
			direction = glm::normalize(input->mouse - glm::vec2{ body->x, body->y });
			AddBullet(gameState, direction, body->x + randomOffsetX, body->y + randomOffsetY, playerControlled->unit.side, playerControlled->unit.attackRange);

			if (std::abs(direction.x) > std::abs(direction.y))
			{
				randomOffsetX = Random::floatInRange(-SHOTGUN_SPREAD, SHOTGUN_SPREAD);
				randomOffsetY = Random::floatInRange(-SHOTGUN_SPREAD_HALF, SHOTGUN_SPREAD_HALF);
			}
			else
			{
				randomOffsetX = Random::floatInRange(-SHOTGUN_SPREAD_HALF, SHOTGUN_SPREAD_HALF);
				randomOffsetY = Random::floatInRange(-SHOTGUN_SPREAD, SHOTGUN_SPREAD);
			}
		}

		setArcherShootAnimation(gameState, direction, body, playerControlled);
	}

	// getBody(playerControlled->guid, body);
	body->x += dirVector.x;
	body->y += dirVector.y;

	static float lastFrameX = 0;

	if (lastFrameX != dirVector.x)
	{
		Anim_enum runAnimation = dirVector.x < 0 ? Anim_Archer_Run_Left : Anim_Archer_Run_Right;
		SetAnimation(gameState, playerControlled->guid, runAnimation);
	}

	if (input->isMouseClicked())
	{
		glm::vec2 direction = glm::normalize(input->mouse - glm::vec2{ body->x, body->y });

		AddBullet(gameState, direction, body->x, body->y, playerControlled->unit.side, playerControlled->unit.attackRange);

		setArcherShootAnimation(gameState, direction, body, playerControlled);
	}

	lastFrameX = dirVector.x;
}

// meleet pitaa seurata

void f(Entity *e, EngineCore* core, PhysicsBody* body)
{

	if (auto player = GET_ENTITY(e, player))
	{
		GetGameState(core);
		DefineInput(core);

		// lista controlloiduista entitytyista id / pointerit
		// tehdaan valintoja sen perusteella rectilla -> vetaa nelion
		// sitten voi kotrolloida uniitteja 

		static bool controllingUnit = false;
		static Entity* controlledEntity = 0;
		if (input->isKeyPressed(SDL_SCANCODE_SPACE))
		{
			if (gameState->selectedCount > 0)
			{
				controllingUnit = !controllingUnit;
				controlledEntity = gameState->selectedEntitys[0];
			}
		}

		if (controllingUnit)
		{
			auto* body = getBody(controlledEntity->guid, gameState->bodies);
			controlUnit(controlledEntity, core, body);
			core->camera2D->setPosition({ body->x, body->y });
			return;
		}

		{
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
		}
		// jos clickkaa / painaa pikanappainta voi rakentaa rakennuksia
		bool dragSelection = input->isMouseDown(1) && player->selectingTroops;
		if (dragSelection)
		{
			glm::vec2 mousePos = input->mouse;
			glm::vec2 startPoint = { player->selectionRect.x, player->selectionRect.y };
			glm::vec2 vector = mousePos - startPoint;
			player->selectionRect.w = vector.x;
			player->selectionRect.h = vector.y;
		}
		else if (input->isMouseClicked(1)) // tutki input juttua enemman
		{
			// valitse -> menusta rakennettavaksi rakennus!
			if (player->selectedBuildingType != building_none)
			{
				// build that building
				buildBuilding(input->mouse.x, input->mouse.y, player->selectedBuildingType, gameState, player->side);

				// player->selectedBuildingType = building_none;
			}
			else
			{
				// ############### start dragging square #####################
				player->selectingTroops = true;
				player->selectionRect.x = input->mouse.x;
				player->selectionRect.y = input->mouse.y;
			}
		}
		else if (input->isMouseClicked(3))
		{
			player->selectedBuildingType = building_none;

			// command dem selected troops / select some troops
			// 1 -> select
			// 2 -> clear selection

			// move or attack here !!!

			std::map<int, std::vector<int>> cachedPaths;

			Uint32 side = gameState->worldmap.GetSideUnderMouse(&input->mouse);
			auto mouseIter = gameState->provinceData.colorToId->find(side);

			int cachedUsed = 0;
			for (int i = 0; i < gameState->selectedCount; i++)
			{
#if 0
				float x = gameState->selectedEntitys[i]->x;
				float y = gameState->selectedEntitys[i]->y;
#else
				float x = (body + gameState->selectedEntitys[i]->guid)->x; // used later!!!!
				float y = (body + gameState->selectedEntitys[i]->guid)->y;
#endif
				Uint32 current = gameState->worldmap.GetPixelSideFromWorld(x, y);
				auto goaliter = gameState->provinceData.colorToId->find(current);

				if (cachedPaths.count(goaliter->second) > 0)
				{
					setPathfindTarget(gameState, cachedPaths.at(goaliter->second), input, i, x, gameState->selectedEntitys[i]);
					gameState->selectedEntitys[i]->unit.path = cachedPaths.find(goaliter->second)->second;
					++cachedUsed;
				}
				else
				{
					if (mouseIter != gameState->provinceData.colorToId->end() && goaliter != gameState->provinceData.colorToId->end())
					{
						std::vector<int> path = BreadthFirst(goaliter->second, gameState->MapNodes, mouseIter->second);

						gameState->selectedEntitys[i]->unit.path = path;
						cachedPaths.insert(std::make_pair(goaliter->second, path));

						setPathfindTarget(gameState, path, input, i, x, gameState->selectedEntitys[i]);
						printf("target set");
					}
					else
					{
						printf("can't set target");
						// ASSERT(false); // why you can pathfind here
					}
				}
			}
			printf("cached paths used %i\n", cachedUsed);
		}

		// CONTROLSSSS
		if (input->isMouseClicked(2))
		{
			Entity* targetEntity = 0;
			Rect rect{ input->mouse.x, input->mouse.y, 60.f, 60.f };

			for (int i = 0; i < gameState->currentEntityCount; i++)
			{
				Entity* e = &gameState->entities[i];
				if (e->type == Entity_unit)
				{
					PhysicsBody* body = (gameState->bodies + i);
					if (rect.Contains(body->x, body->y))
					{
						targetEntity = e;
						break;
					}
				}
			}

			if (targetEntity)
			{
				for (int i = 0; i < gameState->selectedCount; i++)
				{
					// nullaa jos kuollut target
					// gameState->selectedEntitys[i]->unit.attackTarget = targetEntity;
					Entity* selected = gameState->selectedEntitys[i];
					setTargets(selected->guid, gameState, targetEntity);
				}
				printf("target set\n");
			}
		}

		if (input->isKeyPressed(SDL_SCANCODE_G))
		{
			printf("looking for target");
			for (int i = 0; i < gameState->selectedCount; i++)
			{
				Entity* selected = gameState->selectedEntitys[i];
				selected->unit.lookingForTarget = state_lookingForTarget;
			}
		}

		if (input->isKeyPressed(SDL_SCANCODE_C))
		{
			for (int i = 0; i < gameState->currentEntityCount; i++)
			{
				Entity* entity = &gameState->entities[i];
				entity->unit.targetX = -1;
				entity->unit.originalTargetX = -1;
				entity->unit.targetY = -1;
				entity->unit.originalTargetY = -1;

				// entity->unit.attackTarget = nullptr;
				setTargets(entity->guid, gameState, 0);
			}
		}

		bool stoppedSelectingRect = player->selectingTroops && !input->isMouseDown(1);
		if (stoppedSelectingRect)
		{
			player->selectingTroops = false;

			// TODO: choose troops
			player->selectionRect.UseLeftBottomAsStart();

			float leftX = player->selectionRect.x;
			float rigthX = leftX + player->selectionRect.w;

			float topY = player->selectionRect.y;
			float bottomY = topY + player->selectionRect.h;

			auto TopLeft = HashPoint(leftX, topY);
			auto BottomRigth = HashPoint(rigthX, bottomY);

			for (int i = 0; i < 1000; i++)
			{
				gameState->selectedEntitys[i] = nullptr;
			}

			PhysicsBody* selectedBodies[1000]{}; // TODO: tarkista max selected
			allUniques(TopLeft.y, BottomRigth.y, TopLeft.x, BottomRigth.x, gameState->spatialGrid, selectedBodies, 1000);

			int selectedCount = 0;
			for (int i = 0; i < 1000 && selectedBodies[i]; i++)
			{
				Entity* ee = &gameState->entities[selectedBodies[i]->owner];
				gameState->selectedEntitys[selectedCount++] = ee;
			}

			//int selectedCount = 0;
			//for (int i = 0; i < gameState->currentEntityCount && selectedCount < 1000; i++)
			//{
			//	Entity* ee = &gameState->entities[i];
			//	printf("Entitys: %i\n", gameState->currentEntityCount);

			//	if (ee->type == Entity_unit)
			//	{
			//		if (player->selectionRect.Contains((body + ee->guid)->x, (body + ee->guid)->y))
			//		{
			//			if (selectedCount < gameState->maxSelected)
			//				gameState->selectedEntitys[selectedCount++] = ee;
			//		}
			//	}
			//}

			gameState->selectedCount = selectedCount;
			printf("selected %i entitys ", selectedCount);

			player->selectionRect = {};
		}
	}
	else if (auto unit = GET_ENTITY(e, unit))
	{
		GetGameState(core);
		DefineInput(core);

		float x = (body + e->guid)->x;
		float y = (body + e->guid)->y;


		// paljo nopeampi jos soa muodossa kun voisi laittaa targetit + thredaus
		if (unit->lookingForTarget == state_lookingForTarget) // set target // types
		{
			auto gPos = gameState->gridPosition[e->guid];
			auto* grid = gameState->spatialGrid;

			for (int y = -2; y < 3; y++)
			{
				if (gPos.y + y < 0)
					continue;
				for (int x = -2; x < 3; x++)
				{
					if (gPos.x + x < 0)
						continue;

					auto* bodies = &grid->hashMap[gPos.y + y][gPos.x + x];
					for (int i = 0; i < bodies->size(); i++)
					{
						if (BodyToSide(bodies->at(i), gameState) != gameState->allSides[e->guid])
						{
							// e->unit.attackTarget = getEntity(bodies->at(i)->owner, gameState);
							setTargets(e->guid, gameState, getEntity(bodies->at(i)->owner, gameState));
							goto afterLoop1;
						}
					}
				}
			}
		}
	afterLoop1:

		Uint32 mapId = gameState->worldmap.GetPixelSideFromWorld(x, y);
		if (unit->lastFrameProv == mapId)
		{
			// provinssi ei ole vaihtunut! -> jatka liikkumista ampumista jos tarvii
			// -> älä captureta provinssia
		}
		else
		{   // Provinssi on vaihtunut
			// float x = (body + e->guid)->x;
			// float y = (body + e->guid)->y;
			Uint32 side = gameState->worldmap.GetCurrentHolder(x, y);

			if (mapId == 0) // yksikkö tippui veteen
			{
				e->alive = false;
				return;
			}

			if (side != unit->side)
			{
				gameState->worldmap.changeSideWorld(x, y, unit->side, core); // jou
			}

			//if (FollowPath(e, gameState, side)) // laittaa targetin seuraavaan
			//{
			//	// kauas pois
			//	unit->path.pop_back();

			//	printf("pop: ");
			//	printf("(%i, %i)\n", unit->targetX, unit->targetY);

			//}
			//else if (unit->path.size() < 2)
			//{
			//	//if (unit->path.size() > 1)
			//	//{
			//	//	// SetTarget(e, unit->path, 1, &gameState->provinceData);
			//	//	// liiku vapaasti target locatioon?
			//	//}

			//	unit->targetX = unit->originalTargetX;
			//	unit->targetY = unit->originalTargetY;

			//	printf("original \n");
			//}

			// laske path uudestaan jos vaihtuu provinssi muilla tavoilla / ei knockeja!
		}
		unit->lastFrameProv = mapId;

		// attack logic
		unit->mainAttackCD -= core->deltaTime;

		auto attackTarget = getTargets(e->guid, gameState);

		if (attackTarget && unit->mainAttackCD < 0.f) // shoot // attack // Shoot 
		{
#if 1
			// float x = (body + e->guid)->x;
			// float y = (body + e->guid)->y;
#else
			glm::vec2 targetVector{ attackTarget->x - x , attackTarget->y - y };
#endif

			// laske suunta targettiin
			float enemyX = (body + attackTarget->guid)->x;
			float enemyY = (body + attackTarget->guid)->y;
			glm::vec2 targetVector{ enemyX - x, enemyY - y };
			float lengthSquared = targetVector.x * targetVector.x + targetVector.y * targetVector.y;

			if (lengthSquared < unit->attackRange * unit->attackRange) // Attack
			{
				// TODO: tuohon direction/koko offset niin tulee piipusta
				glm::vec2 direction = glm::normalize(targetVector);

				const auto attackType = getAttackType(e->guid, gameState);
				if (attackType == attack_ranged || attackType == attack_zombie)
				{
					AddBullet(gameState, direction, x, y, unit->side, unit->attackRange);

					Anim_enum animation = (x > x + targetVector.x) ? Anim_Archer_Shooting_Left : Anim_Archer_Shooting_Rigth;

					SetAnimation(gameState, e->guid, animation);
				}
				else if (attackType == attack_melee && attackTarget->unit.side != unit->side)
				{
					// melee ?						
					dealDamage(attackTarget, 10);

					Anim_enum animation = (x > x + targetVector.x) ? Anim_Archer_Shooting_Left : Anim_Archer_Shooting_Rigth;
					SetAnimation(gameState, e->guid, animation);

					printf("melee ");
				}

				unit->mainAttackCD = 1.5f;  // lista odottajistaa ?   TODO: CDCDCD!
			}
			else
			{
				unit->originalTargetX = unit->targetX = enemyX;
				unit->originalTargetY = unit->targetY = enemyY;
			}
		}

		if (unit->targetX != -1 && unit->targetY != -1)
		{

			glm::vec2 moveVec{ unit->targetX - x, unit->targetY - y };

			// float length = glm::length(moveVec);
			// korjaa
			float length = /* sqrt */((moveVec.x * moveVec.x + moveVec.y * moveVec.y));
			if (length < 10.f)
			{
				unit->targetX = -1;
				unit->targetY = -1;

				// kauas pois
				// if ( /* FollowPath(e, gameState, mapId) */) 
				if (SetNextPathNode(e, gameState))
				{
					// auto vec2 = 
					unit->path.pop_back();
					printf("pop k");
					//if (unit->path.size() < 2)
					//{
					//	unit->targetX = unit->originalTargetX;
					//	unit->targetY = unit->originalTargetY;
					//}
				}
				else if (unit->path.size() == 2) // ONKO OIKEIN
				{
					printf("origninal target (%i, %i)", unit->originalTargetX, unit->originalTargetY);
					unit->targetX = unit->originalTargetX;
					unit->targetY = unit->originalTargetY;
					// unit->path.pop_back();
					unit->path.clear();

					printf("original \n");
				}
				}
			else
			{
				moveVec = glm::normalize(moveVec) * unit->movementSpeed;
#if 0
				e->x += moveVec.x;
				e->y += moveVec.y;
#else
				(body + e->guid)->x += moveVec.x;
				(body + e->guid)->y += moveVec.y;
#endif

				Uint32 side = gameState->worldmap.GetCurrentHolder((int)x, (int)y);
				static SDL_PixelFormat *fmt = gameState->worldmap.provinces.surface->format;
				Uint8 alpha = getAlpha(side, fmt);
				if (side == 0xFF000000 || alpha == 0) // can't move here
				{
#if 0
					e->x -= moveVec.x;
					e->y -= moveVec.y;
#else
					(body + e->guid)->x -= moveVec.x;
					(body + e->guid)->y -= moveVec.y;
#endif
				}
			}
			}

		}
	else if (auto building = GET_ENTITY(e, building))
	{
		GetGameState(core);
		DefineInput(core);

		building->timer += core->deltaTime;

		if (building->timer > 5.f)
		{
			switch (building->type)
			{
			case building_mill:
			{
				// printf("gimme cahs!\n");
				gameState->player->player.cash += 10;   // tuotanto jutut jne jne paljonko mistakin saa boonuksia jne.. jne..
			} break;
			case building_millitary_factory:
			{
				// printf("gimme troop!\n"

				float x = (body + e->guid)->x;
				float y = (body + e->guid)->y;

				createUnit(x, y, gameState, building->side, attack_ranged, RANGED_RANGE);

			} break;
			default:
				ASSERT(false); // , "building type not found!");
				break;
			}

			building->timer = 0.f;
		}
	}
	}

// tee kunnon spritesheet manager struct jotain jotain...
glm::vec4 getUvFromUp(int index, glm::vec2 dims)
{
	int x = index / dims.x;
	int y = index / dims.x;

	glm::vec4 uvs;

	uvs.x = x / dims.x;
	uvs.y = 1.0f - y / dims.x;

	uvs.z = 1.0f / dims.x;
	uvs.w = 1.0f / dims.y;

	return uvs;
}

glm::vec4 getUVs(int index, glm::vec2 dims)
{
	int xTile = index % (int)dims.x;
	int yTile = index / dims.x;

	glm::vec4 uvs;

	uvs.x = xTile / (float)dims.x;
	uvs.y = 1.0f - ((yTile + 1) / (float)dims.y);
	uvs.z = 1.0f / dims.x;
	uvs.w = 1.0f / dims.y;

	return uvs;
}

void r(Entity *e, EngineCore* core, PhysicsBody* body)
{
	if (auto building = GET_ENTITY(e, building))
	{
		// TODO: korjaa tekstuuri manager
		static GLuint textureId = UpiEngine::ResourceManager::getTexture("building.png").id;

		core->spriteBatch->draw(glm::vec4{ body->x, body->y, 40, 40 }, building->textureUv, textureId, 1.0f);
	}
	else if (auto unit = GET_ENTITY(e, unit))
	{
		Uint32 ucolor = unit->side;
		auto color = Uin32ToColor(ucolor);
		core->spriteBatch->draw(glm::vec4{ body->x - 20, body->y - 20, 40, 40 }, glm::vec4{ 0.f, 0.f, 1.0f, 1.0f }, 3, 1.0f, color);
	}
	else if (auto entity = GET_ENTITY(e, script))
	{
		// debug rect
	}
	else if (auto player = GET_ENTITY(e, player))
	{
		GetGameState(core);
		DefineInput(core);

		char buffer[64];

		sprintf(buffer, "money: %i", player->cash);
		Debug::drawText(buffer, 2);

		if (player->selectingTroops)
			player->selectionRect.DrawRect();
	}
	// else if (auto bullet = GET_ENTITY(e, bullet))
	// {
		// core->spriteBatch->draw(glm::vec4{ e->x, e->y, 20, 20 }, glm::vec4{ 0.f, 0.f, 1.0f, 1.0f }, 3, 1.0f);
	// }
}

Entity* GetFirstAvaibleEntity(game_state* state)
{
	ASSERT(state->currentEntityCount < ArrayCount(state->entities));
	Entity* result = &state->entities[state->currentEntityCount];
	result->guid = state->currentEntityCount++;
	return result;
}

EXPORT __declspec(dllexport) Entity* newEntity(float x, float y, Entity_Enum type, game_state* state)
{
	Entity* result = GetFirstAvaibleEntity(state);

	// result->x = x;
	// result->y = y;

	(state->bodies + result->guid)->x = x;
	(state->bodies + result->guid)->y = y;
	state->entityColors[result->guid] = UpiEngine::ColorRGBA8(255, 255, 255, 255);
	// state->phy

	result->type = type;
	result->alive = true;
	return result;
}


EXPORT __declspec(dllexport) Entity* getById(int i, void* gameState)
{
	auto* state = (game_state*)gameState;
	return &state->entities[i];
}



constexpr float towerRange = 300.f;
constexpr float towerDamage = 15;

const glm::vec2 buildingTextureDims{ 2, 2 };
bool buildBuilding(float x, float  y, building_type type, game_state* state, Uint32 side)
{
	WorldMap* map = &state->worldmap;

	// pelaajaan tuo check
	// map->GetPixelSideFromWorld((float)x, (float)y);
	// vaativa check voiko rakennuksen rakentaa tahan // rakennus bitmap? // probably tarkistetaan vain ymparisto
													  // quad tree alkoi kuulostaa kivalta

	if (type == building_tower)
	{
		Entity* e = createUnit(x, y, state, side, attack_ranged, RANGED_RANGE);
		e->unit.attackRange = towerRange;
		e->unit.mainAttackCD = 1.0f;
		e->unit.movementSpeed = 0.f;

		return true;
	}

	Entity* e = newEntity(x, y, Entity_building, state);
	e->building.type = type;
	e->building.side = side;
	e->building.timer = 0.f;


	switch (type)
	{
	case building_none:
		break;
	case building_millitary_factory:
		e->building.textureUv = getUVs(0, buildingTextureDims);
		break;
	case building_mill:
		e->building.textureUv = getUVs(2, buildingTextureDims);
		break;
	case building_max:
		break;
	default:
		break;
	}
	return true;
}
