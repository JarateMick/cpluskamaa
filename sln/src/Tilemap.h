#pragma once
#include <vector>
#include <SDL2\SDL.h>
#include <SDL2\SDL_image.h>

#ifndef EXPORT
#define EXPORT extern "C"
#endif // !EXPORT

EXPORT __declspec(dllexport) typedef struct
{
	int x;
	int y;
} v2i;

static int TileWidth = 64;
static int TileHeight = 64;

static int TileStepX = 64;
static int TileStepY = 16;

static int OddRowXOffset = 32;
static int OddRowYOffset = 32;
static int HeightTileOffset = 32;

static int squaresAcross = 17;
static int squaresDown = 37;
static int baseOffsetX = -32;
static int baseOffsetY = -64;

static int mapWidth = 50;
static int mapHeight = 50;

static v2i originPoint{ 19, 39 };



class MapCell // TODO(Smartz): preallokoisusta
{
public:
	std::vector<int> baseTiles;
	std::vector<int> heightTiles;
	std::vector<int> topperTiles;


	// TODO: Enums
	int TileID;
	int slopeMap = -1;
	bool walkable = true;

	void AddBaseTile(int tileID)
	{
		baseTiles.push_back(tileID);
	}

	void AddHeightTile(int tileID)
	{
		heightTiles.push_back(tileID);
	}

	void AddTopperTile(int tileID)
	{
		topperTiles.push_back(tileID);
	}


	MapCell(int tileID) : TileID(tileID)
	{
		baseTiles.push_back(TileID);
		slopeMap = -1;
	}

	MapCell(int tileID, bool walkable) : TileID(tileID), walkable(walkable)
	{
		baseTiles.push_back(TileID);
	}

	MapCell() = default;
};

// some surface pixel manipulation helpers
Uint32 get_pixel32(SDL_Surface *surface, int x, int y)
{
	Uint32 *pixels = (Uint32 *)surface->pixels;
	return pixels[(y * surface->w) + x];
}


void put_pixel32(SDL_Surface *surface, int x, int y, Uint32 pixel)
{
	Uint32 *pixels = (Uint32 *)surface->pixels;
	pixels[(y * surface->w) + x] = pixel;
}


v2i WorldCoordinateToMapCell(const glm::vec2& hilightLocation)
{
	static SDL_Surface* surface = IMG_Load("mousemap.png");

	if (surface)
	{
		static bool init = false;
		if (!init)
		{
			SDL_LockSurface(surface);
			init = true;
		}

		v2i mapCell = { (int)(hilightLocation.x / surface->w), (int)(hilightLocation.y / surface->h) * 2 };

		int localPointX = (int)hilightLocation.x % surface->w;
		int localPointY = (int)hilightLocation.y % surface->h;
		int dx = 0;
		int dy = 0;

		// if ( ) POINT CONTAINS

		Uint32 pixel = get_pixel32(surface, localPointX, localPointY);

		if (pixel == 0xFF0000FF) // red
		{
			dx = -1;
			dy = -1;
			localPointX = localPointX + (surface->w / 2);
			localPointY = localPointY + (surface->h / 2);
		}
		if (pixel == 0xFF00FF00) // Green
		{
			dx = -1;
			dy = 1;
			localPointX += surface->w / 2;
			localPointY -= surface->h / 2;
		}
		if (pixel == 0xFF00FFFF) // Yellow
		{
			dy = -1;
			localPointX -= surface->w / 2;
			localPointY += surface->h / 2;
		}
		if (pixel == 0xFFFF0000) // Bluw
		{
			dy = 1;
			localPointX -= surface->w / 2;
			localPointY -= surface->h / 2;
		}
		mapCell.x += dx;
		mapCell.y += dy; // -2;

		// SDL_UnlockSurface(surface);
		// printf("x: %i, %i\n", mapCell.x, mapCell.y);

		return mapCell;
		// v2 veccs((int)hilightLocation.x /  )
	}
	else
	{
		Debug::logError("mousemap.png not found!");
		ASSERT(false);
	}

	return { -1, -1 };
}

v2i WorldCoordinateToMapCell(const glm::vec2& hilightLocation, glm::vec2* outLocalPoint)
{
	static SDL_Surface* surface = IMG_Load("mousemap.png");

	if (surface)
	{
		static bool init = false;

		if (!init)
		{
			SDL_LockSurface(surface);
			init = true;
		}

		v2i mapCell = { (int)(hilightLocation.x / surface->w), (int)(hilightLocation.y / surface->h) * 2 };

		int localPointX = (int)hilightLocation.x % surface->w;
		int localPointY = (int)hilightLocation.y % surface->h;
		int dx = 0;
		int dy = 0;


		// if ( ) POINT CONTAINS

		Uint32 pixel = get_pixel32(surface, localPointX, localPointY);

		if (pixel == 0xFF0000FF) // red
		{
			dx = -1;
			dy = -1;
			localPointX = localPointX + (surface->w / 2);
			localPointY = localPointY + (surface->h / 2);
		}
		if (pixel == 0xFF00FF00) // Green
		{
			dx = -1;
			dy = 1;
			localPointX += surface->w / 2;
			localPointY -= surface->h / 2;
		}
		if (pixel == 0xFF00FFFF) // Yellow
		{
			dy = -1;
			localPointX -= surface->w / 2;
			localPointY += surface->h / 2;
		}
		if (pixel == 0xFFFF0000) // Bluw
		{
			dy = 1;
			localPointX -= surface->w / 2;
			localPointY -= surface->h / 2;
		}
		mapCell.x += dx;
		mapCell.y += dy; // -2;

		outLocalPoint->x = localPointX;
		outLocalPoint->y = localPointY;

		// SDL_UnlockSurface(surface);
		// printf("x: %i, %i\n", mapCell.x, mapCell.y);

		return mapCell;
		// v2 veccs((int)hilightLocation.x /  )
	}
	else
	{
		Debug::logError("mousemap.png not found!");
		ASSERT(false);
	}

	return { -1, -1 };
}


class TileMap
{
public:
	static float heightRowDepthMod;

	int MapWidth = 50;
	int MapHeigth = 50;
#define GetTile(x, y) tiles[(y) * MapWidth + (x)]

	MapCell tiles[50 * 50];

	SDL_Surface* slopeMap = 0;
	SDL_Surface* mouseMap = 0;

	TileMap()
	{
		slopeMap = IMG_Load("slopemap.png");
		mouseMap = IMG_Load("mousemap.png");

		SDL_LockSurface(slopeMap);
		SDL_LockSurface(mouseMap);

		for (int y = 0; y < MapHeigth; y++)
		{
			for (int x = 0; x < MapWidth; x++)
			{
				tiles[y * MapWidth + x] = MapCell(0);
			}
		}

		GetTile(16, 4).AddHeightTile(54);
		GetTile(17, 3).AddHeightTile(54);
		GetTile(15, 3).AddHeightTile(54);

		GetTile(16, 3).AddHeightTile(53);

		GetTile(15, 4).AddHeightTile(54);
		GetTile(15, 4).AddHeightTile(54);

		GetTile(15, 4).AddHeightTile(51);
		GetTile(18, 3).AddHeightTile(51);
		GetTile(19, 3).AddHeightTile(50);
		GetTile(18, 4).AddHeightTile(55);

		GetTile(14, 4).AddHeightTile(54);
		GetTile(14, 5).AddHeightTile(62);
		GetTile(14, 5).AddHeightTile(61);
		GetTile(14, 5).AddHeightTile(63);

		GetTile(17, 4).AddTopperTile(114);
		GetTile(16, 5).AddTopperTile(115);
		GetTile(14, 4).AddTopperTile(125);
		GetTile(15, 5).AddTopperTile(91);
		GetTile(16, 6).AddTopperTile(94);


		GetTile(3, 3).AddHeightTile(54);

		GetTile(7, 7).AddHeightTile(54);
		GetTile(7, 7).AddHeightTile(54);
		GetTile(8, 7).AddHeightTile(54);
		GetTile(8, 7).AddHeightTile(54); // alimmat

		GetTile(6, 8).AddHeightTile(54); // ???
		GetTile(6, 8).AddHeightTile(54);
		GetTile(6, 7).AddHeightTile(54);
		GetTile(6, 7).AddHeightTile(54);

		GetTile(6, 6).AddTopperTile(91); // joki
		GetTile(7, 5).AddTopperTile(94);

		GetTile(8, 4).AddTopperTile(91);
		GetTile(9, 3).AddTopperTile(114);
		GetTile(10, 3).AddTopperTile(114);


		GetTile(30, 15).AddHeightTile(54); //
		GetTile(30, 15).AddHeightTile(54); //
		GetTile(30, 15).AddTopperTile(114); //


		GetTile(31, 15).AddHeightTile(54); //
		GetTile(31, 15).AddHeightTile(54); //
		GetTile(31, 15).AddTopperTile(91); // 


		GetTile(30, 16).AddHeightTile(54); //
		GetTile(30, 16).AddHeightTile(54); //
		GetTile(30, 16).AddTopperTile(114); //


		GetTile(29, 15).AddHeightTile(54); //
		GetTile(29, 15).AddHeightTile(54); //
		GetTile(29, 15).AddTopperTile(114); //
		// SetTile(31, 15).AddHeightTile(54); //
		// SetTile(31, 15).AddHeightTile(54); //
		// SetTile(31, 15).AddTopperTile(114); //



		// ylä-vasen talo:
		GetTile(2, 44).AddHeightTile(54);
		GetTile(2, 44).AddHeightTile(54);
		GetTile(2, 44).AddTopperTile(114);

		GetTile(2, 45).AddHeightTile(54);
		GetTile(2, 45).AddHeightTile(54);
		GetTile(2, 45).AddTopperTile(114);

		GetTile(3, 44).AddHeightTile(54);
		GetTile(3, 44).AddHeightTile(54);
		GetTile(3, 44).AddTopperTile(114);

		GetTile(2, 43).AddHeightTile(54);
		GetTile(2, 43).AddHeightTile(54);
		GetTile(2, 43).AddTopperTile(114);

		// another brick in the wall
		GetTile(3, 42).walkable = false;
		GetTile(3, 42).AddHeightTile(54);

		GetTile(3, 41).walkable = false;
		GetTile(3, 41).AddHeightTile(54);

		GetTile(4, 40).walkable = false;
		GetTile(4, 40).AddHeightTile(54);

		GetTile(4, 39).walkable = false;
		GetTile(4, 39).AddHeightTile(54);

		// A hill to climb!
		GetTile(11, 10).AddHeightTile(34);
		GetTile(11, 9).AddHeightTile(34);
		GetTile(11, 8).AddHeightTile(34);
		GetTile(10, 9).AddHeightTile(34);

		GetTile(10, 8).AddTopperTile(31);
		GetTile(10, 8).slopeMap = 0;
		GetTile(10, 7).AddTopperTile(31);
		GetTile(10, 7).slopeMap = 0;

		GetTile(11, 7).AddTopperTile(32);
		GetTile(11, 7).slopeMap = 1;
		GetTile(12, 8).AddTopperTile(32);
		GetTile(12, 8).slopeMap = 1;

		GetTile(11, 6).AddTopperTile(30);
		GetTile(11, 6).slopeMap = 4;
	}

	static glm::vec2 GetMapCellXY(const v2i &mapCell)
	{
		int hilightrosoffset = 0;
		if (((int)mapCell.y % 2) == 1)
		{
			hilightrosoffset = OddRowXOffset;
		}
		return { (mapCell.x * TileStepX) + hilightrosoffset,
				 (mapCell.y) * TileStepY };
	}

// #define GetTile(x, y) tiles[(y) * MapWidth + (x)]
	void SetTile(int x, int y, int id)
	{
		GetTile(x, y).baseTiles[0] = id;
	}

	int GetSlopeMapHeight(glm::vec2 localPixel, int slopeMap)
	{
		// HUOMI: slope tekstuuri on 'väärin päin'
		v2i texturePoint{ (int)(slopeMap * mouseMap->w + localPixel.x), this->slopeMap->h - (int)localPixel.y };

		// TODO: bounds checking
		// jos rect containaa
		Uint32 pixelColor2 = get_pixel32(this->slopeMap, 95, 13);

		if (true)
		{
			Uint32 pixelColor = get_pixel32(this->slopeMap, texturePoint.x, texturePoint.y);

			int offset = (int)(((255 - (pixelColor & (0xFF))) / 255.f) * HeightTileOffset);
			return offset;
		}

		return 0;
	}

	int GetSlopeHeightAtWorldPoint(glm::vec2 worldPoint)
	{
		glm::vec2 localPoint;
		auto mapPoint = WorldCoordinateToMapCell(worldPoint, &localPoint);

		int slopeMap = GetTile(mapPoint.x, mapPoint.y).slopeMap;

		if (slopeMap == -1) return 0;
		return GetSlopeMapHeight(localPoint, slopeMap);
	}

	// slope height and tile height combined!
	int GetOverallHeight(glm::vec2 worldPoint)
	{
		auto mapCellPoint = WorldCoordinateToMapCell(worldPoint);
		int height = GetTile(mapCellPoint.x, mapCellPoint.y).heightTiles.size() * HeightTileOffset;
		height += GetSlopeHeightAtWorldPoint(worldPoint);

		return height;
	}

	MapCell GetMapCell(int x, int y)
	{
		return GetTile(x, y);
	}

	MapCell GetMapCell(glm::vec2 worldPoint)
	{
		auto vec2 = WorldCoordinateToMapCell(worldPoint);
		return GetTile(vec2.x, vec2.y);
	}

	static float MaxDepth;
};
float TileMap::heightRowDepthMod = 0.00001f;
float TileMap::MaxDepth = ((mapWidth + 1) + ((mapHeight + 1) * TileWidth) * 10);
