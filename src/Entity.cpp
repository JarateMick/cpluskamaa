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
		else if (input->isMouseClicked(2))
		{
			player->selectedBuildingType = building_none;

			// command dem selected troops / select some troops
			// 1 -> select
			// 2 -> clear selection

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

		if (unit->targetX != -1 && unit->targetY != -1)
		{
			glm::vec2 moveVec{ unit->targetX - e->x, unit->targetY - e->y };

			// float length = glm::length(moveVec);
			// korjaa
			float length = /* sqrt */((moveVec.x * moveVec.x + moveVec.y * moveVec.y));
			if (length < 2.f )
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

		/*	glm::vec2 playerMoveVev{ 0.f, 0.f };
			if (input->isKeyDown(SDL_SCANCODE_DOWN))
			{
				playerMoveVev.y -= 1.f;
			}
			if (input->isKeyDown(SDL_SCANCODE_UP))
			{
				playerMoveVev.y += 1.f;
			}
			if (input->isKeyDown(SDL_SCANCODE_LEFT))
			{
				playerMoveVev.x -= 1.f;
			}
			if (input->isKeyDown(SDL_SCANCODE_RIGHT))
			{
				playerMoveVev.x += 1.f;
			}*/
	}
	else if (auto building = GET_ENTITY(e, building))
	{
		GetGameState(core);
		DefineInput(core);

		building->timer += core->deltaTime;

		if (building->timer > 10.f)
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
			} break;
			default:
				ASSERT(false); // , "building type not found!");
				break;
			}

			building->timer = 0.f;
		}
	}
}

//resevallokoi

// var color : uint = 16742927;
// var a : uint = color >> 24 & 255; // 255
// var r : uint = color >> 16 & 255; // 255
// var g : uint = color >> 8 & 255; // 122
// var b : uint = color >> 0 & 255; // 15, wrote this a bit different than above just for example

void r(Entity *e, EngineCore* core)
{
	if (auto entity = GET_ENTITY(e, building))
	{
		core->spriteBatch->draw(glm::vec4{ e->x, e->y, 40, 40 }, glm::vec4{ 0.f, 0.f, 1.0f, 1.0f }, 3, 1.0f);
	}
	else if (auto entity = GET_ENTITY(e, unit))
	{
		Uint32  ucolor = e->unit.side;
		int a = ucolor >> 24 & 255;
		int r = ucolor >> 16 & 255;
		int g = ucolor >> 8  & 255;
		int b = ucolor >> 0  & 255;

		UpiEngine::ColorRGBA8 color(r, g, b, a);
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
		// core->spriteBatch->draw(glm::vec4{ e->x, e->y, 40, 40 }, glm::vec4{ 0.f, 0.f, 1.0f, 1.0f }, 3, 1.0f);
		// Debug::drawTextf(0, "money: %i", player->cash);

		// printf("cash %i", player->cash);

		char buffer[64];
		sprintf(buffer, "money: %i", player->cash);

		Debug::drawText(buffer, 2);

		if (player->selectingTroops)
			player->selectionRect.DrawRect();
		// Debug::drawBox(core->input->mouse.x, core->input->mouse.y, 30.f, 30.f);
	}
}

Entity* GetFirstAvaibleEntity(game_state* state)
{
	ASSERT(state->currentEntityCount < ArrayCount(state->entities));
	return &state->entities[state->currentEntityCount++];
}

Entity* newEntity(float x, float y, Entity_Enum type, game_state* state)
{
	Entity* result = GetFirstAvaibleEntity(state);
	result->x = x;
	result->y = y;
	result->type = type;
	return result;
}

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

	return true;
}
