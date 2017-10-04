#include "SpriteBatch.h"
#include <algorithm>


// #include "util/array.h"


namespace UpiEngine {

	Glyph::Glyph(const glm::vec4 &destRect, const glm::vec4 &uvRect, GLuint Texture, float Depth, const ColorRGBA8 &color) :
		texture(Texture), depth(Depth)
	{

		topLeft.color = color;
		topLeft.setPosition(destRect.x, destRect.y + destRect.w);
		topLeft.setUV(uvRect.x, uvRect.y + uvRect.w);

		bottomLeft.color = color;
		bottomLeft.setPosition(destRect.x, destRect.y);
		bottomLeft.setUV(uvRect.x, uvRect.y);

		bottomRight.color = color;
		bottomRight.setPosition(destRect.x + destRect.z, destRect.y);
		bottomRight.setUV(uvRect.x + uvRect.z, uvRect.y);

		topRight.color = color;
		topRight.setPosition(destRect.x + destRect.z, destRect.y + destRect.w);
		topRight.setUV(uvRect.x + uvRect.z, uvRect.y + uvRect.w);
	}



	Glyph::Glyph(const glm::vec4 &destRect, const glm::vec4 &uvRect, GLuint Texture, float Depth, const ColorRGBA8 &color, float angle) :
		texture(Texture), depth(Depth)
	{
		//xy z on widht  w height
		glm::vec2 halfDims(destRect.z / 2.0f, destRect.w / 2.0f);

		//get the points centered at origin
		glm::vec2 tl(-halfDims.x, halfDims.y);
		glm::vec2 bl(-halfDims.x, -halfDims.y);
		glm::vec2 br(halfDims.x, -halfDims.y);
		glm::vec2 tr(halfDims.x, halfDims.y); // 0, 0, 1, 1

		//rotate the points
		tl = rotatePoint(tl, angle) + halfDims;
		bl = rotatePoint(bl, angle) + halfDims;
		br = rotatePoint(br, angle) + halfDims;
		tr = rotatePoint(tr, angle) + halfDims;

		topLeft.color = color;
		topLeft.setPosition(destRect.x + tl.x, destRect.y + tl.y);
		topLeft.setUV(uvRect.x, uvRect.y + uvRect.w);

		bottomLeft.color = color;
		bottomLeft.setPosition(destRect.x + bl.x, destRect.y + bl.y);
		bottomLeft.setUV(uvRect.x, uvRect.y);

		bottomRight.color = color;
		bottomRight.setPosition(destRect.x + br.x, destRect.y + br.y);
		bottomRight.setUV(uvRect.x + uvRect.z, uvRect.y);

		topRight.color = color;
		topRight.setPosition(destRect.x + tr.x, destRect.y + tr.y);
		topRight.setUV(uvRect.x + uvRect.z, uvRect.y + uvRect.w);
	}

	glm::vec2 Glyph::rotatePoint(glm::vec2 pos, float angle)
	{
		glm::vec2 newv;
		newv.x = pos.x * cos(angle) - pos.y * sin(angle);
		newv.y = pos.x * sin(angle) + pos.y * cos(angle);
		return newv;
	}

	SpriteBatch::SpriteBatch() : _vbo(0), _vao(0)
	{
	}

	SpriteBatch::~SpriteBatch()
	{
	}

	void SpriteBatch::init()
	{
		createVertexArray();
	}

	void SpriteBatch::begin(GlyphSortType sortType /* GlyphSortType::TEXTURE */)
	{
		_sortType = sortType;
		_renderBatches.clear();

		_glyphs.clear();
	}

	void SpriteBatch::end()
	{
		_glyphsPointers.resize(_glyphs.size());

		for (int i = 0; i < _glyphs.size(); i++)
		{
			_glyphsPointers[i] = &_glyphs[i];
		}

		sortGlyphs();
		createRenderBatches();
	}

	void SpriteBatch::draw(const glm::vec4 &destRect, const glm::vec4 &uvRect, GLuint texture, float depth)
	{
		static UpiEngine::ColorRGBA8 defaulColor(255, 255, 255, 255);
		_glyphs.emplace_back(destRect, uvRect, texture, depth, defaulColor);
	}

	void SpriteBatch::draw(const glm::vec4 &destRect, const glm::vec4 &uvRect, GLuint texture, float depth, const ColorRGBA8 &color)
	{
		_glyphs.emplace_back(destRect, uvRect, texture, depth, color);
	}

	void SpriteBatch::draw(const glm::vec4 &destRect, const glm::vec4 &uvRect, GLuint texture, float depth, const ColorRGBA8 &color, float angle)
	{
		_glyphs.emplace_back(destRect, uvRect, texture, depth, color, angle);
	}

	void SpriteBatch::draw(const glm::vec4 &destRect, const glm::vec4 &uvRect, GLuint texture, float depth, const ColorRGBA8 &color, const glm::vec2& dir)
	{
		const glm::vec2 right(1.0f, 0.0f);
		float angle = acos(glm::dot(right, dir));
		if (dir.y < 0.0f) {
			angle = -angle;
		}

		_glyphs.emplace_back(destRect, uvRect, texture, depth, color, angle);
	}

	void SpriteBatch::renderBatch()
	{
		glBindVertexArray(_vao);
		for (int i = 0; i < _renderBatches.size(); i++)
		{
			glBindTexture(GL_TEXTURE_2D, _renderBatches[i].texture);

			glDrawArrays(GL_TRIANGLES, _renderBatches[i].offset, _renderBatches[i].numVertices);
			// glDrawArraysInstanced()
		}
		glBindVertexArray(0);
	}

	// gl_InstanceID -> mikä instanc 0,1,2,3,4

	void createEmptyVbo(int floatCount)
	{
		// int vbo = glad_glGenBuffers()
		// gl bind buffer array buffer 
		// buffer data


	}

	void nulll()
	{
		// vertex Buffer Object

		int amount = 10;

		unsigned int buffer;
		glGenBuffers(1, &buffer);
		glBindBuffer(GL_ARRAY_BUFFER, buffer);
		// glBufferData(GL_ARRAY_BUFFER, amount * sizeof(glm::mat4), &modelMatrices[0], GL_STATIC_DRAW);

		for (unsigned int i = 0; i < rock.meshes.size(); i++)
		{
			unsigned int VAO = rock.meshes[i].VAO;
			glBindVertexArray(VAO);
			// vertex Attributes
			GLsizei vec4Size = sizeof(glm::vec4);
			glEnableVertexAttribArray(3);
			glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)0);
			glEnableVertexAttribArray(4);
			glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(vec4Size));
			glEnableVertexAttribArray(5);

			glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(2 * vec4Size));
			glEnableVertexAttribArray(6);
			glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(3 * vec4Size));

			glVertexAttribDivisor(3, 1);
			glVertexAttribDivisor(4, 1);
			glVertexAttribDivisor(5, 1);
			glVertexAttribDivisor(6, 1);

			glBindVertexArray(0);
		}
	}



	void SpriteBatch::createRenderBatches()
	{
		std::vector <Vertex> vertices;
		vertices.resize(_glyphsPointers.size() * 6);

		if (_glyphsPointers.empty())
		{
			return;
		}

		int offset = 0;
		int cv = 0; /// curretn vertex

		_renderBatches.emplace_back(offset, 6, _glyphsPointers[0]->texture);
		vertices[cv++] = _glyphsPointers[0]->topLeft;
		vertices[cv++] = _glyphsPointers[0]->bottomLeft;
		vertices[cv++] = _glyphsPointers[0]->bottomRight;
		vertices[cv++] = _glyphsPointers[0]->bottomRight;
		vertices[cv++] = _glyphsPointers[0]->topRight;
		vertices[cv++] = _glyphsPointers[0]->topLeft;

		offset += 6;
		// curretn glyph
		for (int cg = 1; cg < _glyphsPointers.size(); cg++)
		{
			if (_glyphsPointers[cg]->texture != _glyphsPointers[cg - 1]->texture)
			{
				_renderBatches.emplace_back(offset, 6, _glyphsPointers[cg]->texture);
			}
			else
			{
				_renderBatches.back().numVertices += 6;
			}
			vertices[cv++] = _glyphsPointers[cg]->topLeft;
			vertices[cv++] = _glyphsPointers[cg]->bottomLeft;
			vertices[cv++] = _glyphsPointers[cg]->bottomRight;
			vertices[cv++] = _glyphsPointers[cg]->bottomRight;
			vertices[cv++] = _glyphsPointers[cg]->topRight;
			vertices[cv++] = _glyphsPointers[cg]->topLeft;
			offset += 6;
		}

		glBindBuffer(GL_ARRAY_BUFFER, _vbo);
		//orphan the buffer
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), nullptr, GL_DYNAMIC_DRAW);
		//upload the data
		glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(Vertex), vertices.data());

		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}


	void SpriteBatch::createVertexArray()
	{
		if (_vao == 0)
		{
			glGenVertexArrays(1, &_vao);
		}
		glBindVertexArray(_vao);

		if (_vbo == 0)
		{
			glGenBuffers(1, &_vbo);
		}
		glBindBuffer(GL_ARRAY_BUFFER, _vbo);

		glEnableVertexAttribArray(0); // kertoo opengl mitä haluamme käyttää eka attribute arrrayta tarvitsee vain yhden koska käytämme vain sijaintia
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);

		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));     // kertoo sijainnin mistä alkaa ja kuinka piirträä pointer juttu dataan vbo jotain

		glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex), (void*)offsetof(Vertex, color)); // kertoo attribute pointerin värin

		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));           // uv atribute pointer

		glBindVertexArray(0);
	}

	void SpriteBatch::sortGlyphs()
	{
		switch (_sortType)
		{
		case GlyphSortType::FRONT_TO_BACK:
			std::stable_sort(_glyphsPointers.begin(), _glyphsPointers.end(), compareFrontToBack);
			break;
		case GlyphSortType::BACK_TO_FRONT:
			std::stable_sort(_glyphsPointers.begin(), _glyphsPointers.end(), compareBackToFront);
			break;
		case GlyphSortType::TEXTURE:
			std::stable_sort(_glyphsPointers.begin(), _glyphsPointers.end(), compareTexture);
			break;
		}
	}


	bool SpriteBatch::compareFrontToBack(Glyph* a, Glyph* b)
	{
		return (a->depth < b->depth);
	}

	bool SpriteBatch::compareBackToFront(Glyph* a, Glyph* b)
	{
		return (a->depth > b->depth);

	}

	bool SpriteBatch::compareTexture(Glyph* a, Glyph* b)
	{
		return (a->texture < b->texture);
	}
}

