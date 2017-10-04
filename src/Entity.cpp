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

// one-to-many      many-to-one

inline void SetTarget(Entity* unit, std::vector<int>& path, int id, ProvinceData* prov)
{
	int targetId = path[id]; // eka on maali
	auto v2 = prov->positions[targetId];
	unit->unit.targetX = v2.x;
	unit->unit.targetY = v2.y;
}

bool FollowPath(Entity* entityUnit, game_state* gameState)
{
	std::vector<int>& path = entityUnit->unit.path;
	if (path.size() > 2)
	{
		// int targetId = path[path.size() - 2]; // eka on maali
		// auto v2 = gameState->provinceData.positions[targetId];
		// entityUnit->unit.targetX = v2.x;
		// entityUnit->unit.targetY = v2.y;

		SetTarget(entityUnit, path, path.size() - 2, &gameState->provinceData);
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

void f(Entity *e, EngineCore* core, PhysicsBody* body)
{

	//if (auto ninja = GET_ENTITY(e, ninja))
	//{
	//	int x{ 0 };
	//	int y{ 0 };
	//	if (core->input->isKeyDown(SDL_SCANCODE_UP))
	//		--y;
	//	if (core->input->isKeyDown(SDL_SCANCODE_DOWN))
	//		++y;
	//	if (core->input->isKeyDown(SDL_SCANCODE_LEFT))
	//		--x;
	//	if (core->input->isKeyDown(SDL_SCANCODE_RIGHT))
	//		++x;

	//	if (x != 0 || y != 0)
	//	{
	//		// glm::vec2 normal = glm::normalize(glm::vec2{x, y});
	//		// e->pos.x += (normal.x * core->deltaTime * 0.5f);
	//		// e->pos.y += (normal.y * core->deltaTime * 0.5f);
	//		e->x += (float)x * core->deltaTime * 25.f;
	//		e->y += (float)y * core->deltaTime * 25.f;
	//		//printf("%f", y * core->deltaTime * 0.5f);
	//	}
	//	//e->y += sinf(e->x) * 5;
	//}
	//else if (auto npc = GET_ENTITY(e, npc))
	//{
	//	if (core->input->isKeyPressed(SDL_SCANCODE_SPACE))
	//	{
	//		// printf("ninjas are gays!\n");
	//	}
	//}
	//if (auto entity = GET_ENTITY(e, script))
	//{
	//	GetGameState(core);
	//	DefineInput(core);

	//	// nuista clikattavista voisi laittaa jonnekin mukavammin saataville core->clicables
	//	// tai sitten tekisi entity updatesta systeemi maisen

	//	//if (core->input->isMouseClicked(1))
	//	//{
	//	//	for (int i = 0; i < gameState->currentEntityCount; i++) {

	//	//		Entity& e = gameState->entities[i];
	//	//		if (e.type == Entity_script)
	//	//		{
	//	//			auto script = GET_ENTITY(&e, script);
	//	//			//if (script->hitbox.Contains(input->mouse.x, input->mouse.y))
	//	//			{
	//	//				// hitbox clicked! show message
	//	//			}
	//	//		}
	//	//	}
	//	//}
	//}
	if (auto player = GET_ENTITY(e, player))
	{
		GetGameState(core);
		DefineInput(core);



		// lista controlloiduista entitytyista id / pointerit
		// tehdaan valintoja sen perusteella rectilla -> vetaa nelion
		// sitten voi kotrolloida uniitteja 

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

			for (int i = 0; i < gameState->selectedCount; i++)
			{
#if 0
				float x = gameState->selectedEntitys[i]->x;
				float y = gameState->selectedEntitys[i]->y;
#else
				float x = (body + gameState->selectedEntitys[i]->guid)->x;
				float y = (body + gameState->selectedEntitys[i]->guid)->y;
#endif

				Uint32 side = gameState->worldmap.GetSideUnderMouse(&input->mouse);
				Uint32 current = gameState->worldmap.GetPixelSideFromWorld(x, y);
				auto mouseIter = gameState->provinceData.colorToId->find(side);
				auto goaliter = gameState->provinceData.colorToId->find(current);

				if (mouseIter != gameState->provinceData.colorToId->end() && goaliter != gameState->provinceData.colorToId->end())
				{
					std::vector<int> path = BreadthFirst(goaliter->second, gameState->MapNodes, mouseIter->second);

					gameState->selectedEntitys[i]->unit.path = path;
					if (path.size() > 2)
					{
						int targetId = path[path.size() - 2]; // eka on maali
						auto v2 = gameState->provinceData.positions[targetId];
						gameState->selectedEntitys[i]->unit.targetX = v2.x;
						gameState->selectedEntitys[i]->unit.targetY = v2.y;

						gameState->selectedEntitys[i]->unit.originalTargetX = input->mouse.x;
						gameState->selectedEntitys[i]->unit.originalTargetY = input->mouse.y;

						printf("lyhy (%i, %i)\n", (int)input->mouse.x, (int)input->mouse.y);
						printf("matka\n"); // seuraa reittiä
					}
					else  // mee viereiseen
					{
						int targetId = path[0];
						auto v2 = gameState->provinceData.positions[targetId];
						gameState->selectedEntitys[i]->unit.targetX = v2.x;
						gameState->selectedEntitys[i]->unit.targetY = v2.y;

						gameState->selectedEntitys[i]->unit.targetX = input->mouse.x;
						gameState->selectedEntitys[i]->unit.targetY = input->mouse.y;
						gameState->selectedEntitys[i]->unit.originalTargetX = input->mouse.x;
						gameState->selectedEntitys[i]->unit.originalTargetY = input->mouse.y;
						printf("lyhy (%i, %i)\n", (int)input->mouse.x, (int)input->mouse.y);
					}

					printf("target set");
				}
				else
				{
					printf("can't set target");
					// ASSERT(false); // why you can pathfind here
				}
			}

			if (input->isMouseClicked(2))
			{
				// set targettoroo!
			}

		}

		bool stoppedSelectingRect = player->selectingTroops && !input->isMouseDown(1);
		if (stoppedSelectingRect)
		{
			printf("SELECTION END\n");
			player->selectingTroops = false;

			// TODO: choose troops
			player->selectionRect.UseLeftBottomAsStart();

			for (int i = 0; i < 1000; i++)
			{
				gameState->selectedEntitys[i] = nullptr;
			}

			int selectedCount = 0;
			for (int i = 0; i < gameState->currentEntityCount; i++)
			{
				Entity* ee = &gameState->entities[i];
				printf("Entitys: %i\n", gameState->currentEntityCount);

				if (ee->type == Entity_unit)
				{
					if (player->selectionRect.Contains((body + ee->guid)->x, (body + ee->guid)->y))
					{
						if (selectedCount < gameState->maxSelected)
							gameState->selectedEntitys[selectedCount++] = ee;
					}
				}
			}

			gameState->selectedCount = selectedCount;
			printf("selected %i entitys ", selectedCount);
		}
	}
	else if (auto unit = GET_ENTITY(e, unit))
	{
		GetGameState(core);
		DefineInput(core);

		float x = (body + e->guid)->x;
		float y = (body + e->guid)->y;

		Uint32 mapId = gameState->worldmap.GetPixelSideFromWorld(x, y);
		if (unit->lastFrameProv == mapId)
		{
			// provinssi ei ole vaihtunut! -> jatka liikkumista ampumista jos tarvii
			// -> älä captureta provinssia
		}
		else
		{   // Provinssi on vaihtunut
#if 0
			Uint32 side = gameState->worldmap.GetCurrentHolder((int)e->x, (int)e->y);
#else

			// float x = (body + e->guid)->x;
			// float y = (body + e->guid)->y;
			Uint32 side = gameState->worldmap.GetCurrentHolder(x, y);
#endif

			if (side != unit->side)
			{
				gameState->worldmap.changeSideWorld(x, y, unit->side, core); // jou
			}

			if (FollowPath(e, gameState)) // laittaa targetin seuraavaan
			{
				// kauas pois
				unit->path.pop_back();
				printf("pop: ");
				printf("(%i, %i)\n", unit->targetX, unit->targetY);
			}
			else
			{
				//if (unit->path.size() > 1)
				//{
				//	// SetTarget(e, unit->path, 1, &gameState->provinceData);
				//	// liiku vapaasti target locatioon?
				//}
				unit->targetX = unit->originalTargetX;
				unit->targetY = unit->originalTargetY;
			}
			// laske path uudestaan jos vaihtuu provinssi muilla tavoilla / ei knockeja!
		}
		unit->lastFrameProv = mapId;

		// attack logic
		unit->mainAttackCD -= core->deltaTime;
		auto attackTarget = unit->attackTarget;

		if (attackTarget && unit->mainAttackCD < 0.f) // shoot // attack // Shoot 
		{
#if 1
			// float x = (body + e->guid)->x;
			// float y = (body + e->guid)->y;
#else
			glm::vec2 targetVector{ attackTarget->x - x , attackTarget->y - y };
#endif

			// laske suunta targettiin
			glm::vec2 targetVector{ (body + attackTarget->guid)->x - x, (body + attackTarget->guid)->y - y };

			float lengthSquared = targetVector.x * targetVector.x + targetVector.y * targetVector.y;

			if (lengthSquared < unit->attackRange * unit->attackRange)
			{
				// TODO: tuohon direction/koko offset niin tulee piipusta
#if 0 
				Entity* bulletEntity = newEntity(e->x, e->y, Entity_bullet, gameState);

				glm::vec2 direction = glm::normalize(targetVector);
				bulletEntity->velX = direction.x;
				bulletEntity->velY = direction.y;
				bulletEntity->bullet.rangeSquared = unit->attackRange * unit->attackRange;
				bulletEntity->bullet.speed = 2.f;
				bulletEntity->bullet.startX = e->x;
				bulletEntity->bullet.startY = e->y;
#else
				glm::vec2 direction = glm::normalize(targetVector);

				BulletBody body;
				body.position.x = x; // e->x
				body.position.y = y; // e->y
				body.r = BULLET_BASE_SIZE;
				body.side = unit->side;

				BulletStart start;
				start.position = body.position;
				start.rangeSqrt = unit->attackRange * unit->attackRange;

				gameState->bulletBodies[gameState->bulletCount] = body;
				gameState->BulletAccelerations[gameState->bulletCount] = direction;
				gameState->bulletStart[gameState->bulletCount] = start;

				++gameState->bulletCount;


				// printf("bullet at start fo shoting %i \n", gameState->bulletCount);

#endif

				// bulletit checkkaa collisionit ite 
				unit->mainAttackCD = 1.5f;  // lista odottajistaa ?
			}
		}

		if (unit->targetX != -1 && unit->targetY != -1)
		{

			glm::vec2 moveVec{ unit->targetX - x, unit->targetY - y };

			// float length = glm::length(moveVec);
			// korjaa
			float length = /* sqrt */((moveVec.x * moveVec.x + moveVec.y * moveVec.y));
			if (length < 2.f)
			{
				unit->targetX = -1;
				unit->targetY = -1;

				// kauas pois
				if (FollowPath(e, gameState)) // laittaa targetin seuraavaan
				{
					unit->path.pop_back();
					printf("pop");
				}
				else if (unit->path.size() > 0)
				{
					printf("origninal target (%i, %i)", unit->originalTargetX, unit->originalTargetY);
					unit->targetX = unit->originalTargetX;
					unit->targetY = unit->originalTargetY;
					unit->path.pop_back();
				}
			}
			else
			{
				moveVec = glm::normalize(moveVec) * UNIT_SPEED;          // unit->moveSpeed; mitä on tapahtunut move speedille :-(
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
#if 1
				Entity *ee = newEntity(x + Random::floatInRange(-25.f, 25.f), y + Random::floatInRange(-25.f, 25.f), Entity_unit, gameState);
				(body + ee->guid)->r = 15.f;
				(body + ee->guid)->owner = ee->guid;
#else
				Entity *ee = newEntity(e->x, e->y - 15.f, Entity_unit, gameState);
#endif
				// printf("%f, %f", ee->x, ee->y);
				ee->unit.attackRange = 250.f;
				ee->unit.targetX = -1;
				ee->unit.targetY = -1;
				ee->unit.originalTargetX = -1;
				ee->unit.originalTargetY = -1;
				ee->unit.side = building->side;
				ee->unit.hp = UNIT_BASE_HP;
				gameState->allSides[ee->guid] = building->side; // tarkka kenen guid fuck

			} break;
			default:
				ASSERT(false); // , "building type not found!");
				break;
			}

			building->timer = 0.f;
		}
	}

	// depricated:
	//else if (auto bullet = GET_ENTITY(e, bullet))
	//{
	//	GetGameState(core);
	//	// DefineInput(core);

	//	// jos pelista saisi 100% determistisen niin matkan voisi 
	//	// varmaan kalkuloida suoraan alussa frammeina

	//	e->x += e->velX * core->deltaTime * bullet->speed; // speedx
	//	e->y += e->velY * core->deltaTime * bullet->speed; // speedx

	//	float x = e->x - bullet->startX;
	//	float y = e->y - bullet->startY;
	//	if (abs(x * x + y * y) > bullet->rangeSquared)
	//	{
	//		// max range reached!
	//		e->alive = false;
	//		printf("olen donezo\n");
	//	}
	//}
}

UpiEngine::ColorRGBA8 Uin32ToColor(Uint32 color)
{
	GLubyte a = color >> 24 & 255;
	GLubyte r = color >> 16 & 255;
	GLubyte g = color >> 8 & 255;
	GLubyte b = color >> 0 & 255;
	return UpiEngine::ColorRGBA8(b, g, r, a); // uint32 on muodossa ABGR
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


const glm::vec2 buildingTextureDims{ 2, 2 };
bool buildBuilding(float x, float  y, building_type type, game_state* state, Uint32 side)
{
	WorldMap* map = &state->worldmap;

	// pelaajaan tuo check
	// map->GetPixelSideFromWorld((float)x, (float)y);
	// vaativa check voiko rakennuksen rakentaa tahan // rakennus bitmap? // probably tarkistetaan vain ymparisto
													  // quad tree alkoi kuulostaa kivalta
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
