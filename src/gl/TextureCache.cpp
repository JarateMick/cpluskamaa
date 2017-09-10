#include "TextureCache.h"
#include "ImageLoader.h"
#include <iostream>

namespace UpiEngine{
	TextureCache::TextureCache()
	{
	}


	TextureCache::~TextureCache()
	{
	}

	GLTexture TextureCache::getTexture(std::string texturePath)
	{
		//std::<std::string, GLTexture>::iterator auto tekee saman
		//katsoo onko tekstuuri mapissa
		auto mit = _textureMap.find(texturePath);


		//tarkistaa jos se ei ole mapissa
		if (mit == _textureMap.end())
		{
			GLTexture newTexture = ImageLoader::loadPNG(texturePath);

			//parit
			//std::pair<std::string, GLTexture> newPair(texturePath, newTexture);
			//make_pair tekee saman kuin tämä

			//laittaa parin ppiiin inserttaa
			_textureMap.insert(make_pair(texturePath, newTexture));

			std::cout << "loaded texture!\n";
			return newTexture;
		}

		std::cout << "loaded cached texture!\n";
		return mit->second;
	}
}