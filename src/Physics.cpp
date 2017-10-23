#include "Physics.h"

static inline PhysicsBody* getBody(int id, PhysicsBody* bodies)
{
	return (bodies + id);
}

void AddBodyToGrid(PhysicsBody* body, SpatialHash* hash, GridPosition* positions)
{
	auto v2 = HashPoint(body->x, body->y);
	hash->hashMap[v2.y][v2.x].push_back(body);
	positions->x = v2.x;
	positions->y = v2.y;
}

void SwapBody(int gridX, int gridY, int newGridX, int newGridY, PhysicsBody* bodyToSwap, SpatialHash* hash)
{
	auto* vector = &hash->hashMap[gridY][gridX];
#if 0
	vector->erase(std::remove(vector->begin(), vector->end(), bodyToSwap), vector->end());
#else
	int size = vector->size();
	for (int i = 0; i < size; i++)
	{
		if (vector->at(i) == bodyToSwap)
		{
			(*vector)[i] = vector->at(size - 1);
			vector->pop_back();
			break;
		}
	}
#endif
	hash->hashMap[newGridY][newGridX].push_back(bodyToSwap);
}

void UpdateAllGridPosition(GridPosition* positions, PhysicsBody* bodies, SpatialHash* hasher, const int count)
{
	for (int i = 5; i < count; i++)
	{
		auto v2 = HashPoint((bodies + i)->x, (bodies + i)->y);
		if (v2.x != positions[i].x || v2.y != positions[i].y)
		{
			SwapBody(positions[i].x, positions[i].y, v2.x, v2.y, bodies + i, hasher);
			positions[i].x = v2.x;
			positions[i].y = v2.y;
		}
	}
}

void allUniques(int startY, int endY, int startX, int endX, SpatialHash* hash, PhysicsBody* bodiesOut[], int outSize)
{
	int count = 0;
	for (int i = startY; i < endY && count != outSize; i++)
	{
		for (int j = startX; j < endX; j++)
		{
			std::vector<PhysicsBody*>* v = &hash->hashMap[i][j];
			std::sort(v->begin(), v->end()); // 1 1 2 2 3 3 3 4 4 5 5 6 7 
			const auto last = std::unique(v->begin(), v->end());
			v->erase(last, v->end()); // vain samoja

			for (PhysicsBody* value : *v)
			{
				if (count < outSize)
				{
					bodiesOut[count] = value;
					count++;
				}
			}
		}
	}
}

void CheckCollision(int start, std::vector<PhysicsBody*>* bodies, PhysicsBody* __restrict body)
{
	for (int i = start; i < bodies->size(); i++)
	{
		PhysicsBody* body2 = bodies->at(i);

		glm::vec2 centerPosA = glm::vec2{ body->x, body->y } +glm::vec2(body->r);
		const float MIN_DISTANCE = body->r + body2->r;  // molempiend dist
		glm::vec2 centerPosB = glm::vec2{ body2->x, body2->y } +glm::vec2(body2->r);
		glm::vec2 distVec = centerPosA - centerPosB;

		if (distVec.x * distVec.x + distVec.y * distVec.y < MIN_DISTANCE * MIN_DISTANCE)
		{
			const glm::vec2 collisionDepthVec = glm::normalize(distVec) * 7.5f; // test
			const glm::vec2 aResolution = collisionDepthVec / 2.f; // +=
			body->x += aResolution.x;
			body->y += aResolution.y;

			const glm::vec2 bResolution = collisionDepthVec / 2.0f;  // -=
			body2->x -= bResolution.x;
			body2->y -= bResolution.y;
		}
	}
}

static inline bool isLeft(int x, float positionX)
{
	return (positionX - (x * cellSize) < 7.5f);
}

int CheckCollisions(SpatialHash* hash)
{
	int count = 0;

#pragma omp parallel for schedule(dynamic, 2) num_threads(3)
	for (int i = 0; i < CellsY; i++) // y
	{
		// #pragma omp paraller for schedule(dynamic)
		for (int j = 0; j < CellsX; j++) // x
		{
			std::vector<PhysicsBody*>* bodies = &hash->hashMap[i][j];
			for (int k = 0; k < bodies->size(); k++)
			{
				auto* body = hash->hashMap[i][j].at(k);
				CheckCollision(k + 1, &hash->hashMap[i][j], body);

				if (j > 0)
				{
					//if (isLeft(j, body->x))
					CheckCollision(0, &hash->hashMap[i][j - 1], body);

					if (i > 0)
					{
						CheckCollision(0, &hash->hashMap[i - 1][j - 1], body);
					}

					if (i < CellsY - 1)
					{
						CheckCollision(0, &hash->hashMap[i + 1][j - 1], body);
					}
				}
				if (i > 0)
				{
					CheckCollision(0, &hash->hashMap[i - 1][j], body);
				}
			}
		}
	}
	return count;
}

bool circleCollision(BulletBody* __restrict a, PhysicsBody* __restrict b)
{
	const float MIN_DISTANCE = a->r + b->r;

	glm::vec2 centerPosA = glm::vec2{ a->position.x, a->position.y } +glm::vec2(a->r);
	glm::vec2 centerPosB = glm::vec2{ b->x, b->y } +glm::vec2(b->r);
	glm::vec2 distVec = centerPosA - centerPosB;

	const float distance = glm::length(distVec);
	const float collisionDepth = MIN_DISTANCE - distance;

	return (collisionDepth > 0);
}


void circleCollision(PhysicsBody* __restrict a, PhysicsBody* __restrict b)
{
#if 0
	const float MIN_DISTANCE = a->r * 2.0f;
	glm::vec2 centerPosA = glm::vec2{ a->x, a->y } +glm::vec2(a->r);
	glm::vec2 centerPosB = glm::vec2{ b->x, b->y } +glm::vec2(b->r);
	glm::vec2 distVec = centerPosA - centerPosB;


	// const float distance = glm::length(distVec);
	const float collisionDepth = MIN_DISTANCE - distance;
#else
	const float MIN_DISTANCE = a->r + b->r;  // molempiend dist
	glm::vec2 centerPosA = glm::vec2{ a->x, a->y } +glm::vec2(a->r);
	glm::vec2 centerPosB = glm::vec2{ b->x, b->y } +glm::vec2(b->r);
	glm::vec2 distVec = centerPosA - centerPosB;

	// const float distance = glm::length(distVec);
	// const float collisionDepth = MIN_DISTANCE - distance;
#endif
	if (distVec.x * distVec.x + distVec.y * distVec.y < MIN_DISTANCE * MIN_DISTANCE)
	{
		//	iconst float collisionDepth = 6.f;
		// const float distance = 2.f;
		/*if (distance == 0.f)
		{
			a->x += 30.f;
			a->y += 30.f;
			b->x -= 30.f;
			b->y -= 30.f;
			return;
		}*/
		// const float collisionDepth =  MIN_DISTANCE - glm::length(distVec);
		const glm::vec2 collisionDepthVec = glm::normalize(distVec) * 7.5f; // test
		if (collisionDepthVec.x < 0.1f && collisionDepthVec.y < 0.1f)
		{
			const glm::vec2 aResolution = collisionDepthVec * 2.f; // +=
			a->x += aResolution.x;
			a->y += aResolution.y;

			const glm::vec2 bResolution = collisionDepthVec * 2.0f;  // -=
			b->x -= bResolution.x;
			b->y -= bResolution.y;
			return;
		}

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

		// agent->_position -= collisionDepthVec / 2.0f;
#endif
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

int simulateBullets(BulletBody* bodies, vec2f* bulletAcceceration, BulletStart* start, int count)
{
	for (int i = count - 1; i > -1; i--)
	{
		bodies[i].position += bulletAcceceration[i] * 4.f; // TODO: MIKA ON LUODIN NOPEUS JOUJOUJOU!!!

		// printf("bullet (%f, %f)\n", bodies[i].position.x, bodies[i].position.y);

		float x = bodies[i].position.x - start[i].position.x;
		float y = bodies[i].position.y - start[i].position.y;

		if (abs(x * x + y * y) > start[i].rangeSqrt)
		{
			// luoti kuoli! poista kaikki kerralla?
			bodies[i] = bodies[count - 1];
			start[i] = start[count - 1];
			bulletAcceceration[i] = bulletAcceceration[count - 1];
			--count;
		}
	}
	return count;
}


void swapAll(int i, int count, Bullets* bullets)
{
	BulletBody*  bodies = bullets->bodies;
	BulletStart* start = bullets->start;
	vec2f*       acc = bullets->accelerations;

	bodies[i] = bodies[count - 1];
	start[i] = start[count - 1];
	acc[i] = acc[count - 1];
}


// struct BulletDmgOut { int targetId, attackerId; };
int collideBullets(SpatialHash* hash, BulletBody* bulletBodies, Uint32* sides, int size, Bullets* bullets, std::vector<int>* damageOut)
{
	for (int i = size - 1; i > -1; i--)
	{
		BulletBody* body = bulletBodies + i;

		auto point = HashPoint(body->position.x, body->position.y);

		if (point.x > 0)
		{
			auto* bodies = &hash->hashMap[point.y][point.x];
			for (int j = 0; j < bodies->size(); j++)
			{
				PhysicsBody* physicsBody = bodies->at(j);
				if (sides[physicsBody->owner] != body->side)
				{
					if (circleCollision(body, physicsBody))
					{
						damageOut->push_back(physicsBody->owner);
						swapAll(i, size, bullets);

						--size;
						break;
					}
				}
			}
		}
	}
	return size;
}

inline v2 HashPoint(int x, int y)
{
	return{ x / cellSize, y / cellSize };
}

inline v2 HashPoint(v2 v)
{
	return{ v.x / cellSize, v.y / cellSize };
}
