#pragma once

// #include <GL/glew.h>
#include <glad\glad.h>
#include <glm\glm.hpp>

#include "Vertex.h"
#include <vector>

namespace UpiEngine {

	enum class GlyphSortType {
		NONE,
		FRONT_TO_BACK,
		BACK_TO_FRONT,
		TEXTURE
	};

	class Glyph {
	public:
		Glyph() {};
		Glyph(const glm::vec4 &destRect, const glm::vec4 &uvRect, GLuint Texture, float Depth, const ColorRGBA8 &color);
		Glyph(const glm::vec4 &destRect, const glm::vec4 &uvRect, GLuint Texture, float Depth, const ColorRGBA8 &color, float angle);

		GLuint texture;
		float depth;

		Vertex topLeft;
		Vertex bottomLeft;
		Vertex topRight;
		Vertex bottomRight;
	private:
		glm::vec2 rotatePoint(glm::vec2 pos, float angle);
	};

	class RenderBatch {
	public:
		RenderBatch(GLuint Offset, GLuint NumVerticles, GLuint Texture) : offset(Offset), numVertices(NumVerticles), texture(Texture)
		{	}
		GLuint offset;
		GLuint numVertices;
		GLuint texture;
	};

	class SpriteBatch
	{
	public:
		SpriteBatch();
		~SpriteBatch();

		void init();

		void begin(GlyphSortType _sortType = GlyphSortType::TEXTURE);
		void end();

		void draw(const glm::vec4 &destRect, const glm::vec4 &uvRect, GLuint texture, float depth);
		void draw(const glm::vec4 &destRect, const glm::vec4 &uvRect, GLuint texture, float depth, const ColorRGBA8 &color);
		void draw(const glm::vec4 &destRect, const glm::vec4 &uvRect, GLuint texture, float depth, const ColorRGBA8 &color, float angle);
		void draw(const glm::vec4 &destRect, const glm::vec4 &uvRect, GLuint texture, float depth, const ColorRGBA8 &color, const glm::vec2 &dir);


		void renderBatch();


	private:
		void createRenderBatches();
		void createVertexArray();
		void sortGlyphs();

		static bool compareFrontToBack(Glyph* a, Glyph* b);
		static bool compareBackToFront(Glyph* a, Glyph* b);
		static bool compareTexture(Glyph* a, Glyph* b);

		GLuint _vbo;
		GLuint _vao;

		std::vector<Glyph> _glyphs; // < nämä on aidot käytettävät glyphit
		std::vector<Glyph*> _glyphsPointers; // < sorttaamista vartem

		std::vector<RenderBatch> _renderBatches;

		GlyphSortType _sortType;
	};
}
