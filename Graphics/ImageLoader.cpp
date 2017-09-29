#include "ImageLoader.h"
#include "picoPNG.h"
#include "IOManager.h"
// #include "UpiEngineErrors.h"

namespace UpiEngine{
	GLTexture ImageLoader::loadPNG(std::string filePath)
	{
		GLTexture texture = {};

		std::vector<unsigned char> in;
		std::vector<unsigned char> out;

		unsigned long widht, heigth;

		if (IOManager::readFileToBuffer(filePath, in) == false)
		{
			// fatalError("Failed to load PNG file to buffer");
			printf("Failed to load PNG file to buffer");
		}

		int errorCode = decodePNG(out, widht, heigth, &(in[0]), in.size());
		if (errorCode != 0)
		{
			// fatalError("decodePNG failed with error: " + std::to_string(errorCode));
			printf("decodePNG failed with error: %d", errorCode);
		}

		glGenTextures(1, &(texture.id)); //generoi textuurin
		glBindTexture(GL_TEXTURE_2D, texture.id);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, widht, heigth, 0, GL_RGBA, GL_UNSIGNED_BYTE, &(out[0])); //jotain textuuri kamaa näytlnohjain

		//hmmmmn commentoiduilla blendaaa mutta näillä saa aidot bitti grafiikat!!!!
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

		
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		
		glGenerateMipmap(GL_TEXTURE_2D);

		glBindTexture(GL_TEXTURE_2D, 0);

		texture.widht = widht;
		texture.height = heigth;

		return texture;
	}
}