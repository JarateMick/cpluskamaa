#pragma once

#include "GLSLProgram.h"
#include "Vertex.h"
#include "SpriteFont.h"

#include <glm/glm.hpp>
#include <vector>

struct v2_d {
	float x, y;
};

namespace UpiEngine {


	class DebugRenderer
	{
	public:
		DebugRenderer();
		~DebugRenderer();

		void init();
		void end();
		void render(const glm::mat4& projectionMatrix, float lineWidth);
		void dispose();

		void drawBox(const glm::vec4& destRect, const ColorRGBA8& color, float angle);
		void drawCircle(const glm::vec2& center, const ColorRGBA8& color, float radius);

		void drawLine(float x1, float y1, float x2, float y2, const ColorRGBA8& color);
		void drawLine(v2_d start, v2_d end, const ColorRGBA8& color);
		void drawLine(glm::vec2& start, glm::vec2 end, const ColorRGBA8& color);

		// void drawText(const char* buffer, glm::vec2 position, glm::vec2 scaling, float depth, ColorRGBA8 tint);
		void drawText(char* buffer, int slot);

		void _dDrawSlots(SpriteBatch& b);

		struct DebugVertex {
			glm::vec2 position;
			UpiEngine::ColorRGBA8 color;
		};


		const char* debugTTFfontName = "rs.ttf";
	private:
#define bufferSize 128
		struct DebugTextSlot {
			char buffer[bufferSize];
		};


		UpiEngine::GLSLProgram  m_program;
		UpiEngine::SpriteFont   m_font;
		// UpiEngine::SpriteBatch* m_fontBatch;

		std::vector<DebugVertex> m_verts;
		std::vector<GLuint> m_indices;
		GLuint m_vbo = 0, m_vao = 0, m_ibo = 0;
		int m_numElements = 0;

		const int               m_debugFontSize = 20;
		DebugTextSlot           m_textSlots[4];
		int                     m_usedTextSlots;
	};
}

