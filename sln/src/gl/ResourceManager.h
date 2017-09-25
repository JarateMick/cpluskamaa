#pragma once
#include "TextureCache.h"
#include <string>

namespace UpiEngine {

	class ResourceManager
	{
	public:
		static GLTexture getTexture(std::string texturePath);

		static void init();
		static TextureCache* GetContext();
		static void SetContext(TextureCache* cache);
	private:
		static TextureCache _textureCache;
		static TextureCache* contextPtr;
	};
}
