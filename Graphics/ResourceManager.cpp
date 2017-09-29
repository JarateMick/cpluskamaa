#include "ResourceManager.h"

namespace UpiEngine 
{

	TextureCache ResourceManager::_textureCache;
	TextureCache* ResourceManager::contextPtr = 0;

	 void ResourceManager::init()
	{
		contextPtr = &_textureCache;
	}

	GLTexture ResourceManager::getTexture(std::string texturePath)
	{
		return contextPtr->getTexture(texturePath);
	}

	void ResourceManager::SetContext(TextureCache* cache)
	{
		contextPtr = cache;
	}
	TextureCache* ResourceManager::GetContext()
	{
		return contextPtr;
	}

}