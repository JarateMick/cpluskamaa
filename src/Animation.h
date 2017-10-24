enum Anim_enum
{
	Anim_Unit,

	Anim_Archer_Shooting_Rigth,
	Anim_Archer_Run_Right,     // hahahahaha hassu tapa
	Anim_Archer_Shooting_Left,
	Anim_Archer_Run_Left,     // hahahahaha hassu tapa

	Anim_Archer_default,
	Anim_Archer_looping,

	Anim_Melee_Run_Left,
	Anim_Melee_Attack_Left,
	Anim_Melee_Run_Rigth,
	Anim_Melee_Attack_Rigth,
};

enum Anim_types
{
	Anim_type_looping,
	Anim_type_to_next,
	// Anim_type_to_idle, // ?????????
};

// animaatiot olisi hash map array entityihin ja se päättäisi      1 1 1 1 1 1 1, 2 22 2 2, 3 33 
// tyyliin animaatiot / cd:t

constexpr int MAX_ENTITY_COUNT = 80000;
struct Animations
{
	glm::vec4  uvs[MAX_ENTITY_COUNT];
	int        frameTimes[MAX_ENTITY_COUNT];
	int        currentAnimationFrame[MAX_ENTITY_COUNT]; // vaihtuu joka frametime 0:ssa

	Anim_enum  animations[MAX_ENTITY_COUNT];     // pakkohan sen on olla tyyppi kun muuten ei tied' miten kasavateta
	Anim_types animationTypes[MAX_ENTITY_COUNT];
};

// framecount
inline int GetAnimationTime(Anim_enum anim)
{
	switch (anim)
	{
	case Anim_Unit: return 5;  break;
	case Anim_Archer_default: return 1337; break;
	case Anim_Archer_looping: return 8; break;
	case Anim_Archer_Run_Right:  return 5; break;
	case Anim_Archer_Shooting_Rigth:  return 5; break;
	case Anim_Archer_Shooting_Left:  return 5; break;
	default: return 1000000;
	}
}

// frame -> uv 

// framen UV
// framen vaihto aika seuraavaan
// tamanhetkinen frame

// haluaisin sano uv rectin + ajan

					   //              0.08f
//  1.0f  /  count    = local uv.x 
//  1.0f  /  count    = local uv.y


constexpr float COUNTW = 12.f;
constexpr float COUNTH = 1.f;
constexpr float frameW = 1.0f / COUNTW;
constexpr float frameH = 1.0f / COUNTH;



//constexpr f4 getFrameUvss(int index, glm::vec2 dims)
//{
//	int xTile = index % (int)dims.x;
//	int yTile = index / dims.x;
//
//	glm::vec4 uvs;
//
//	uvs.x = xTile / (float)dims.x;
//	uvs.y = 1.0f - ((yTile + 1) / (float)dims.y);
//	uvs.z = 1.0f / dims.x;
//	uvs.w = 1.0f / dims.y;
//
//	return { 1, 2, 3, 4 };
//}

constexpr float SIZE_X = 12;
constexpr float SIZE_Y = 4;

#define TileX(index) (int)((index) % (int)SIZE_X)
#define TileY(index) (int)((index) / SIZE_X)

#define getFrameUvsm(index) { TileX(index) / SIZE_X, 1.0f - ((TileY(index) + 1.f) / (float)SIZE_Y), 1.0f / SIZE_X, 1.0f / SIZE_Y }

#define getFrameUv(indexX, indexY, textureW, textureH, offsetX, offsetY, perOneWidth, perOneHeight) \
	{ (((offsetX) + ((indexX) * (perOneWidth)))    / (float)(textureW)), \
	   1.0f - ((offsetY + (perOneHeight) * (indexY)) / (float)(textureH)), \
	  ((perOneWidth) / (textureW)), \
	  ((perOneHeight) / (textureH)) \
	}

// sizePerOne_x sizePerOne_y

// 426 x 100
// 12  x 4      -> laske per 1
// offset 10 x 5
// 35.5 x 25
// aloita offsetista 
// (value-min)/(max-min) 
// 730 x  407
glm::vec4 uvtest(int indexX, int indexY, int textureW, int textureH, int offsetX, int offsetY, float perOneWidth, float perOneHeight)
{
	glm::vec4 uvs;

	uvs.x = (offsetX + indexX * perOneWidth ) / textureW;
	uvs.y = 1.0f - (offsetY + indexY * perOneHeight) / textureH;

	uvs.z = (perOneWidth / textureW);
	uvs.w = (perOneHeight / textureH);
	
	return uvs;
}

//static glm::vec4 archerRunLeft[12] = { 
//	 {  frameW * 0, 0.f, frameW, 1.0f }, { frameW * 1, 0.f, frameW , 1.0f },
//	 {  frameW * 2,  0.f, frameW , 1.0f }, { frameW * 3, 0.f, frameW , 1.0f },
//	 {  frameW * 4, 0.f, frameW , 1.0f }, { frameW * 5, 0.f, frameW , 1.0f },
//	 {  frameW * 6, 0.f, frameW , 1.0f }, { frameW * 7, 0.f, frameW , 1.0f },
//	 {  frameW * 8, 0.f, frameW , 1.0f }, { frameW * 9, 0.f, frameW , 1.0f },
//	 {  frameW * 10, 0.f, frameW , 1.0f }, { frameW * 11, 0.f, frameW , 1.0f } };
// 

//static glm::vec4 archerRunLeft[11] = {
//	getFrameUvsm(23),
//	getFrameUvsm(24),
//	getFrameUvsm(25),
//	getFrameUvsm(26),
//	getFrameUvsm(27),
//	getFrameUvsm(28),
//	getFrameUvsm(29),
//	getFrameUvsm(30),
//	getFrameUvsm(31),
//	getFrameUvsm(32),
//	getFrameUvsm(33),
//	// getFrameUvsm(11),
//};

// 

static glm::vec4 archerRunLeft[11] = {
	// getFrameUvsm(35),
	// getFrameUvsm(36),
	// getFrameUvsm(37),
	// getFrameUvsm(38),
	// getFrameUvsm(39),
	// getFrameUvsm(40),
	// getFrameUvsm(41),
	// getFrameUvsm(42),
	// getFrameUvsm(43),
	// getFrameUvsm(44),
	// getFrameUvsm(45),
	// getFrameUvsm(11),
	getFrameUv( 11,  -1, 730.f, 407.f, 10.f, 105.f, 36.f, 25.f),
	getFrameUv( 0, -0, 730.f, 407.f, 10.f, 105.f, 36.f, 25.f),
	getFrameUv( 1, -0, 730.f, 407.f, 10.f, 105.f, 36.f, 25.f),
	getFrameUv( 2, -0, 730.f, 407.f, 10.f, 105.f, 36.f, 25.f),
	getFrameUv( 3, -0, 730.f, 407.f, 10.f, 105.f, 36.f, 25.f),
	getFrameUv( 4, -0, 730.f, 407.f, 10.f, 105.f, 36.f, 25.f),
	getFrameUv( 5, -0, 730.f, 407.f, 10.f, 105.f, 36.f, 25.f),
	getFrameUv( 6, -0, 730.f, 407.f, 10.f, 105.f, 36.f, 25.f),
	getFrameUv( 7, -0, 730.f, 407.f, 10.f, 105.f, 36.f, 25.f),
	getFrameUv( 8, -0, 730.f, 407.f, 10.f, 105.f, 36.f, 25.f),
	getFrameUv( 9, -0, 730.f, 407.f, 10.f, 105.f, 36.f, 25.f),
	// getFrameUv( 10, -1, 730, 407, 10.f, 105.f, 36.f, 25.f),
};

constexpr float meleeTexH = 21.f;
constexpr float meleeTexW = 36.f;
static glm::vec4 archerRunRight[12] = {

	getFrameUv( 11,  -2, 730, 407, 10.f, 105.f, 36.f, 25.f),
	 getFrameUv( 0, -1, 730, 407, 10.f, 105.f, 36.f, 25.f),
	 getFrameUv( 1, -1, 730, 407, 10.f, 105.f, 36.f, 25.f),
	 getFrameUv( 2, -1, 730, 407, 10.f, 105.f, 36.f, 25.f),
	 getFrameUv( 3, -1, 730, 407, 10.f, 105.f, 36.f, 25.f),
	 getFrameUv( 4, -1, 730, 407, 10.f, 105.f, 36.f, 25.f),
	 getFrameUv( 5, -1, 730, 407, 10.f, 105.f, 36.f, 25.f),
	 getFrameUv( 6, -1, 730, 407, 10.f, 105.f, 36.f, 25.f),
	 getFrameUv( 7, -1, 730, 407, 10.f, 105.f, 36.f, 25.f),
	 getFrameUv( 8, -1, 730, 407, 10.f, 105.f, 36.f, 25.f),
	 getFrameUv( 9, -1, 730, 407, 10.f, 105.f, 36.f, 25.f),
	 getFrameUv( 10, -1, 730, 407, 10.f, 105.f, 36.f, 25.f),

};


// melee attackit ^^
static glm::vec4 meleeAttackLeft[2] = 
{
    getFrameUv( 4.f, -1.f, 730.f, 407.f, 11.f,  199.f, meleeTexW, meleeTexH - 0.5f),
	getFrameUv( 0.f, -3.f, 730.f, 407.f, 11.f,  199.f, meleeTexW, meleeTexH),
};

static glm::vec4 meleeAttackRigth[2] = 
{
    getFrameUv( 3.f, -1.f, 730.f, 407.f, 11.f,  199.f, meleeTexW, meleeTexH - 1.f),
	getFrameUv( 4.f, -0.f, 730.f, 407.f, 11.f,  199.f, meleeTexW, meleeTexH - 1.f),
};

// meleeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee
static glm::vec4 meleeRunningRight[12] = {
	getFrameUv( 1, -2, 730, 407, 12.f, 198.f, 36.f,  meleeTexH - 0.5f),
	getFrameUv( 2, -2, 730, 407, 12.f, 198.f, 36.f,  meleeTexH - 0.5f),
	getFrameUv( 3, -2, 730, 407, 12.f, 198.f, 36.f,  meleeTexH - 0.5f),
	getFrameUv( 4, -2, 730, 407, 12.f, 198.f, 36.f,  meleeTexH - 0.5f),
	getFrameUv( 5, -2, 730, 407, 12.f, 198.f, 36.f,  meleeTexH - 0.5f),
	getFrameUv( 6, -2, 730, 407, 12.f, 198.f, 36.f,  meleeTexH - 0.5f),
	getFrameUv( 7, -2, 730, 407, 12.f, 198.f, 36.f,  meleeTexH - 0.5f),
	getFrameUv( 8, -2, 730, 407, 12.f, 198.f, 36.f,  meleeTexH - 0.5f),
	getFrameUv( 9, -2, 730, 407, 12.f, 198.f, 36.f,  meleeTexH - 0.5f),
	getFrameUv( 10, -2, 730, 407, 12.f, 198.f, 36.f, meleeTexH - 0.5f),
	getFrameUv( 11, -2, 730, 407, 12.f, 198.f, 36.f, meleeTexH - 0.5f),
	getFrameUv( 0, -1, 730, 407, 12.f, 198.f, 36.f,  meleeTexH - 0.5f),
};

static glm::vec4 meleeRunningLeft[12] = {
	getFrameUv( 1, -3, 730, 407, 12.f, 198.f, 36.f, 21.f),
	getFrameUv( 2, -3, 730, 407, 12.f, 198.f, 36.f, 21.f),
	getFrameUv( 3, -3, 730, 407, 12.f, 198.f, 36.f, 21.f),
	getFrameUv( 4, -3, 730, 407, 12.f, 198.f, 36.f, 21.f),
	getFrameUv( 5, -3, 730, 407, 12.f, 198.f, 36.f, 21.f),
	getFrameUv( 6, -3, 730, 407, 12.f, 198.f, 36.f, 21.f),
	getFrameUv( 7, -3, 730, 407, 12.f, 198.f, 36.f, 21.f),
	getFrameUv( 8, -3, 730, 407, 12.f, 198.f, 36.f, 21.f),
	getFrameUv( 9, -3, 730, 407, 12.f, 198.f, 36.f, 21.f),
	getFrameUv( 10, -3, 730, 407, 12.f, 198.f, 36.f, 21.f),
	getFrameUv( 11, -3, 730, 407, 12.f, 198.f, 36.f, 21.f),
	getFrameUv( 0, -2, 730, 407, 12.f, 198.f, 36.f, 21.f),
};

//  35-36 x 21
//  12    x 198


static glm::vec4 archerShootingLeft[2] = {
	getFrameUv( 5, -3, 730, 407, 10.f, 105.f, 35.5f, 25.f),
	getFrameUv( 10, -3, 730, 407, 10.f, 105.f, 35.5f, 25.f),
};

// static glm::vec4 archerShootingLeft[2] =
// {
	// getFrameUvsm(5), getFrameUvsm(10),
// };

static glm::vec4 archerShootingRigth[2] =
{
	getFrameUv( 6, -3, 730, 407, 10.f, 105.f, 35.5f, 25.f),
	getFrameUv( 9, -3, 730, 407, 10.f, 105.f, 35.5f, 25.f),
//	getFrameUvsm(6), getFrameUvsm(8),
};

//static glm::vec4 meleeRunRight[12]
//static glm::vec4 meleeRunRight[12]
// static glm::vec4 meleeRunRight[12]

//#define ImplementAnimation(arrayName, frame) \
//	if (*frame == ArrayCount(arrayName)) \
//	{ \
//		*frame = 0; \
//    } \
//
//#define ImplementAnimationNext(arrayName, AnimationFrame, nextAnim) \
//	ImplementAnimation(arrayName, AnimationFrame) \
//    
    

glm::vec4 GetNextAnim(Anim_enum* type, int* AnimationFrame) // muuttaa jos tarvii seuraavaan ei looppaaaavaan animaation
{
	if (*type == Anim_Archer_Run_Right)
	{
		if (*AnimationFrame == ArrayCount(archerRunRight))
		{
			(*AnimationFrame) = 0;
		}
		return archerRunRight[*AnimationFrame];
	}
	else if (*type == Anim_Archer_Run_Left)
	{
		if (*AnimationFrame == ArrayCount(archerRunLeft))
		{
			(*AnimationFrame) = 0;
		}
		return archerRunLeft[*AnimationFrame];
	}
	else if (*type == Anim_Archer_Shooting_Rigth)
	{
		if (*AnimationFrame == ArrayCount(archerShootingRigth))
		{
			*type = Anim_Archer_Run_Right;
			(*AnimationFrame) = 0;
		}
		return archerShootingRigth[*AnimationFrame];
	}
	else if (*type == Anim_Archer_Shooting_Left)
	{
		if (*AnimationFrame == ArrayCount(archerShootingLeft))
		{
			*type = Anim_Archer_Run_Left;
			(*AnimationFrame) = 0;
		}
		return archerShootingLeft[*AnimationFrame];
	}
	else if (*type == Anim_Melee_Run_Left)
	{
		if (*AnimationFrame == ArrayCount(meleeRunningLeft))
		{
			(*AnimationFrame) = 0;
		}
		return meleeRunningLeft[*AnimationFrame];
	}
	else if (*type == Anim_Melee_Attack_Left)
	{
		if (*AnimationFrame == ArrayCount(meleeAttackLeft))
		{
			(*AnimationFrame) = 0;
			*type = Anim_Melee_Run_Left;
		}
		return meleeAttackLeft[*AnimationFrame];
	}
	else if (*type == Anim_Melee_Run_Rigth)
	{
		if (*AnimationFrame == ArrayCount(meleeRunningRight))
		{
			(*AnimationFrame) = 0;
		}
		return meleeRunningRight[*AnimationFrame];
	}
	else if (*type == Anim_Melee_Attack_Rigth)
	{
		if (*AnimationFrame == ArrayCount(meleeAttackRigth))
		{
			(*AnimationFrame) = 0;
			*type = Anim_Melee_Run_Rigth;
		}
		return meleeAttackRigth[*AnimationFrame];
	}

	*AnimationFrame = 0;
	return archerRunLeft[*AnimationFrame]; // who is default animation;
}


void UpdateAnimations(Animations* anims, const int count)
{
	for (int i = 0; i < count; i++)
	{
		anims->frameTimes[i] -= 1;

		if (anims->frameTimes[i] < 0)
		{
			anims->currentAnimationFrame[i] += 1;
			anims->frameTimes[i] = 7;
			anims->uvs[i] = GetNextAnim(&anims->animations[i], &anims->currentAnimationFrame[i]);

			// set next animation (if animation is attack or smt....)
			// or reset animTime
			//switch (anims->animationTypes[i])
			//{
			//case Anim_type_looping:
			//{
			//	anims->frameTimes[i] = GetAnimationTime(anims->animations[i]);
			//} break;
			//case Anim_type_to_next:
			//{
			//	anims->animations[i] = (Anim_enum)(anims->animationTypes[i] + 1);
			//	anims->frameTimes[i] = GetAnimationTime(anims->animations[i]);
			//} break;
			//case Anim_type_to_idle:
			//{

			//} break;
		}
	}
}
