#pragma once

#include "GLTexture.h"
#include <glm/glm.hpp>

namespace UpiEngine {

	class TileSheet {
	public:
		void init(const GLTexture& texture, const glm::ivec2& tileDims)
		{
			this->texture = texture;
			this->dims = tileDims;
		}

		// x / texturesize = uv.x
		// y / texturesize = uv.y

		// index                    0  1  2  3 
		//                          4  5  6  7
		//                          8  9  10 11
		//                          12 13 14 15

		glm::vec4 getUvFromUp(int index)
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

		glm::vec4 getUVs(int index)
		{
			int xTile = index % dims.x;
			int yTile = index / dims.x;

			glm::vec4 uvs;
			
			uvs.x = xTile / (float)dims.x;
			uvs.y = 1.0f - ((yTile + 1) / (float)dims.y);
			uvs.z =  1.0f / dims.x;
			uvs.w =  1.0f / dims.y;

			return uvs;
		}

		//glm::vec4 GetSourceRectangle(int tileIndex, float TileWidth, float TileHeight)
		//{
		//	int tileY = tileIndex / (texture.widht / TileWidth);
		//	int tileX = tileIndex % (texture.height / TileWidth);
		//	return { tileX * TileWidth, tileY * TileHeight, TileWidth, TileHeight };
		//}

		glm::vec4 getUVs(int x, int y)
		{
			glm::vec4 uvs;
			uvs.x = x / (float)dims.x;
			uvs.y = y / (float)dims.y;
			uvs.z = 1.0f / dims.x;
			uvs.w = 1.0f / dims.y;
			return uvs;
		}

		GLTexture texture;
		glm::ivec2 dims;
	};
}
