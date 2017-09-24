#include "Entity.h"
#include "core.h"
#include "game.h"

void f(Entity *e, EngineCore* core)
{
	if (auto ninja = GET_ENTITY(e, ninja))
	{
		int x{ 0 };
		int y{ 0 };
		if (core->input->isKeyDown(SDL_SCANCODE_UP))
			--y;
		if (core->input->isKeyDown(SDL_SCANCODE_DOWN))
			++y;
		if (core->input->isKeyDown(SDL_SCANCODE_LEFT))
			--x;
		if (core->input->isKeyDown(SDL_SCANCODE_RIGHT))
			++x;

		if (x != 0 || y != 0)
		{
			// glm::vec2 normal = glm::normalize(glm::vec2{x, y});
			// e->pos.x += (normal.x * core->deltaTime * 0.5f);
			// e->pos.y += (normal.y * core->deltaTime * 0.5f);
			e->x += (float)x * core->deltaTime * 25.f;
			e->y += (float)y * core->deltaTime * 25.f;
			//printf("%f", y * core->deltaTime * 0.5f);
		}
		//e->y += sinf(e->x) * 5;
	}
	else if (auto npc = GET_ENTITY(e, npc))
	{
		if (core->input->isKeyPressed(SDL_SCANCODE_SPACE))
		{
			// printf("ninjas are gays!\n");
		}
	}
	else if (auto entity = GET_ENTITY(e, script))
	{
		GetGameState(core);
		DefineInput(core);

		// nuista clikattavista voisi laittaa jonnekin mukavammin saataville core->clicables
		// tai sitten tekisi entity updatesta systeemi maisen

		//if (core->input->isMouseClicked(1))
		//{
		//	for (int i = 0; i < gameState->currentEntityCount; i++) {

		//		Entity& e = gameState->entities[i];
		//		if (e.type == Entity_script)
		//		{
		//			auto script = GET_ENTITY(&e, script);
		//			//if (script->hitbox.Contains(input->mouse.x, input->mouse.y))
		//			{
		//				// hitbox clicked! show message
		//			}
		//		}
		//	}
		//}
	}
	else if (auto player = GET_ENTITY(e, player))
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

				player->selectedBuildingType = building_none;
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
				gameState->selectedEntitys[i]->unit.targetX = input->mouse.x;
				gameState->selectedEntitys[i]->unit.targetY = input->mouse.y;
			}
			printf("target set");
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
				Entity* e = &gameState->entities[i];
				if (e->type == Entity_unit)
				{
					if (player->selectionRect.Contains(e->x, e->y))
					{
						if (selectedCount < gameState->maxSelected)
							gameState->selectedEntitys[selectedCount++] = e;
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

		// if (unit->side == gameState.playerSide)
		// {
		// }
		Uint32 side = gameState->worldmap.GetCurrentHolder((int)e->x, (int)e->y);
		if (side != unit->side)
		{
			gameState->worldmap.changeSideWorld((int)e->x, (int)e->y, unit->side, core);
		}

		// attack logic
		unit->mainAttackCD -= core->deltaTime;
		auto attackTarget = unit->attackTarget;

		if ((attackTarget) && unit->mainAttackCD < 0.f) // SHOOT // attack
		{
			// laske suunta targettiin
			glm::vec2 targetVector{ attackTarget->x - e->x , attackTarget->y - e->y };
			float lengthSquared = targetVector.x * targetVector.x + targetVector.y * targetVector.y;

			if (lengthSquared < unit->attackRange * unit->attackRange)
			{
				// TODO: tuohon direction/koko offset niin tulee piipusta
				Entity* bulletEntity = newEntity(e->x, e->y, Entity_bullet, gameState);

				glm::vec2 direction = glm::normalize(targetVector);
				bulletEntity->velX = direction.x;
				bulletEntity->velY = direction.y;
				bulletEntity->bullet.rangeSquared = unit->attackRange * unit->attackRange;
				bulletEntity->bullet.speed = 2.f;
				bulletEntity->bullet.startX = e->x;
				bulletEntity->bullet.startY = e->y;

				unit->mainAttackCD = 50.f;
			}
		}

		if (unit->targetX != -1 && unit->targetY != -1)
		{
			glm::vec2 moveVec{ unit->targetX - e->x, unit->targetY - e->y };

			// float length = glm::length(moveVec);
			// korjaa
			float length = /* sqrt */((moveVec.x * moveVec.x + moveVec.y * moveVec.y));
			if (length < 2.f)
			{
				unit->targetX = -1;
				unit->targetY = -1;
			}
			else
			{
				moveVec = glm::normalize(moveVec) * 0.1f * 1.f;
				e->x += moveVec.x;
				e->y += moveVec.y;

				Uint32 side = gameState->worldmap.GetCurrentHolder((int)e->x, (int)e->y);
				static SDL_PixelFormat *fmt = gameState->worldmap.provinces.surface->format;
				Uint8 alpha = getAlpha(side, fmt);
				if (side == 0xFF000000 || alpha == 0) // can't move here
				{
					e->x -= moveVec.x;
					e->y -= moveVec.y;
				}
			}
		}
	}
	else if (auto building = GET_ENTITY(e, building))
	{
		GetGameState(core);
		DefineInput(core);

		building->timer += core->deltaTime;

		if (building->timer > 40.f)
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
				// printf("gimme troop!\n");
				Entity *ee = newEntity(e->x + Random::floatInRange(-25.f, 25.f), e->y + Random::floatInRange(-25.f, 25.f), Entity_unit, gameState);
				ee->unit.side = building->side;
			} break;
			default:
				ASSERT(false); // , "building type not found!");
				break;
			}

			building->timer = 0.f;
		}
	}
	else if (auto bullet = GET_ENTITY(e, bullet))
	{
		GetGameState(core);
		// DefineInput(core);

		// jos pelista saisi 100% determistisen niin matkan voisi 
		// varmaan kalkuloida suoraan alussa frammeina

		e->x += e->velX * core->deltaTime * bullet->speed; // speedx
		e->y += e->velY * core->deltaTime * bullet->speed; // speedx

		float x = e->x - bullet->startX;
		float y = e->y - bullet->startY;
		if (abs(x * x + y * y) > bullet->rangeSquared)
		{
			// max range reached!
			e->alive = false;
			printf("olen donezo\n");
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


void r(Entity *e, EngineCore* core)
{
	if (auto building = GET_ENTITY(e, building))
	{
		// TODO: korjaa tekstuuri manager
		static GLuint textureId = UpiEngine::ResourceManager::getTexture("building.png").id;
		core->spriteBatch->draw(glm::vec4{ e->x, e->y, 40, 40 }, building->textureUv, textureId, 1.0f);
	}
	else if (auto unit = GET_ENTITY(e, unit))
	{
		Uint32 ucolor = unit->side;
		auto color = Uin32ToColor(ucolor);
		core->spriteBatch->draw(glm::vec4{ e->x, e->y, 40, 40 }, glm::vec4{ 0.f, 0.f, 1.0f, 1.0f }, 3, 1.0f, color);
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
	else if (auto bullet = GET_ENTITY(e, bullet))
	{
		core->spriteBatch->draw(glm::vec4{ e->x, e->y, 20, 20 }, glm::vec4{ 0.f, 0.f, 1.0f, 1.0f }, 3, 1.0f);
	}
}

Entity* GetFirstAvaibleEntity(game_state* state)
{
	ASSERT(state->currentEntityCount < ArrayCount(state->entities));
	return &state->entities[state->currentEntityCount++];
}

EXPORT __declspec(dllexport) Entity* newEntity(float x, float y, Entity_Enum type, game_state* state) 
{
	Entity* result = GetFirstAvaibleEntity(state);
	result->x = x;
	result->y = y;
	result->type = type;
	result->alive = true;
	return result;
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
