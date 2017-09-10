#pragma once
#include <glad\glad.h>
#include "GLTexture.h"
#include <string>

namespace UpiEngine {
	class Sprite
	{
	public:
		Sprite();
		~Sprite();

		void init(float x, float y, float width, float height, std::string texturePath);
		void draw();


	private:
		float _x;
		float _y;
		float _width;
		float _heigth;
		GLuint _vboID; // vertex buffer     GL unsigned int
		GLTexture _texture;
	};

}