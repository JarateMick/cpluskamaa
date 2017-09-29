#pragma once
#include "GLTexture.h"
#include <string>

namespace UpiEngine {
	class ImageLoader
	{
	public:
		static GLTexture loadPNG(std::string filePath);
	};

}
