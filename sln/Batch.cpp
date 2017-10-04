#include <glad/glad.h>
#include "Batch.h"
#include <string>
#include <queue>
#include <memory>

Batch::Batch(unsigned uMaxNumVertices) :
	_uMaxNumVertices(uMaxNumVertices),
	_uNumUsedVertices(0),
	_vao(0),
	_vbo(0),
	_config(GL_TRIANGLE_STRIP, 0, 0),
	_lastVertex(glm::vec2(), glm::vec4())
{
	//optimal size for a batch is between 1-4MB in size. Number of elements that can be stored in a
	//batch is determined by calculating #bytes used by each vertex
	if (uMaxNumVertices < 1000) {
		std::ostringstream strStream;
		strStream << __FUNCTION__ << " uMaxNumVertices{" << uMaxNumVertices << "} is too small. Choose a number >= 1000 ";
		throw ExceptionHandler(strStream);
	}
	//clear error codes
	glGetError();
	if (Settings::getOpenglVersion().x >= 3) {
		glGenVertexArrays(1, &_vao);
		glBindVertexArray(_vao);
	}
	//create batch buffer
	glGenBuffers(1, &_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, _vbo);
	glBufferData(GL_ARRAY_BUFFER, uMaxNumVertices * sizeof(GuiVertex), nullptr, GL_STREAM_DRAW);
	if (Settings::getOpenglVersion().x >= 3) {
		unsigned uOffset = 0;
		ShaderManager::enableAttribute(A_POSITION, sizeof(GuiVertex), uOffset);
		uOffset += sizeof(glm::vec2);
		ShaderManager::enableAttribute(A_COLOR, sizeof(GuiVertex), uOffset);
		uOffset += sizeof(glm::vec4);
		ShaderManager::enableAttribute(A_TEXTURE_COORD0, sizeof(GuiVertex), uOffset);
		glBindVertexArray(0);
		ShaderManager::disableAttribute(A_POSITION);
		ShaderManager::disableAttribute(A_COLOR);
		ShaderManager::disableAttribute(A_TEXTURE_COORD0);
	}
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	if (GL_NO_ERROR != glGetError()) {
		cleanUp();
		throw ExceptionHandler(__FUNCTION__ + std::string(" failed to create batch"));
	}
}//Batch
//------------------------------------------------------------------------
Batch::~Batch() {
	cleanUp();
}//~Batch
//------------------------------------------------------------------------
void Batch::cleanUp() {
	if (_vbo != 0) {
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glDeleteBuffers(1, &_vbo);
		_vbo = 0;
	}
	if (_vao != 0) {
		glBindVertexArray(0);
		glDeleteVertexArrays(1, &_vao);
		_vao = 0;
	}
}//cleanUp
//------------------------------------------------------------------------
bool Batch::isBatchConfig(const BatchConfig& config) const {
	return (config == _config);
}//isBatchConfig
//------------------------------------------------------------------------
bool Batch::isEmpty() const {
	return (0 == _uNumUsedVertices);
}//isEmpty
//------------------------------------------------------------------------
//returns true if the number of vertices passed in can be stored in this batch
//without reaching the limit of how many vertices can fit in the batch
bool Batch::isEnoughRoom(unsigned uNumVertices) const {
	//2 extra vertices are needed for degenerate triangles between each strip
	unsigned uNumExtraVertices = (GL_TRIANGLE_STRIP == _config.uRenderType && _uNumUsedVertices > 0 ? 2 : 0);
	return (_uNumUsedVertices + uNumExtraVertices + uNumVertices <= _uMaxNumVertices);
}//isEnoughRoom
//------------------------------------------------------------------------
//returns the batch that contains the most number of stored vertices between
//this batch and the one passed in
Batch* Batch::getFullest(Batch* pBatch) {
	return (_uNumUsedVertices > pBatch->_uNumUsedVertices ? this : pBatch);
}//getFullest
//------------------------------------------------------------------------
int Batch::getPriority() const {
	return _config.iPriority;
}//getPriority
//------------------------------------------------------------------------
//adds vertices to batch and also sets the batch config options
void Batch::add(const std::vector& vVertices, const BatchConfig& config) {
	_config = config;
	add(vVertices);
}//add
//------------------------------------------------------------------------
void Batch::add(const std::vector& vVertices) {
	//2 extra vertices are needed for degenerate triangles between each strip
	unsigned uNumExtraVertices = (GL_TRIANGLE_STRIP == _config.uRenderType && _uNumUsedVertices > 0 ? 2 : 0);
	if (uNumExtraVertices + vVertices.size() > _uMaxNumVertices - _uNumUsedVertices) {
		std::ostringstream strStream;
		strStream << __FUNCTION__ << " not enough room for {" << vVertices.size() << "} vertices in this batch. Maximum number of vertices allowed in a batch is {" << _uMaxNumVertices << "} and {" << _uNumUsedVertices << "} are already used";
		if (uNumExtraVertices > 0) {
			strStream << " plus you need room for {" << uNumExtraVertices << "} extra vertices too";
		}
		throw ExceptionHandler(strStream);
	}
	if (vVertices.size() > _uMaxNumVertices) {
		std::ostringstream strStream;
		strStream << __FUNCTION__ << " can not add {" << vVertices.size() << "} vertices to batch. Maximum number of vertices allowed in a batch is {" << _uMaxNumVertices << "}";
		throw ExceptionHandler(strStream);
	}
	if (vVertices.empty()) {
		std::ostringstream strStream;
		strStream << __FUNCTION__ << " can not add {" << vVertices.size() << "} vertices to batch.";
		throw ExceptionHandler(strStream);
	}
	//add vertices to buffer
	if (Settings::getOpenglVersion().x >= 3) {
		glBindVertexArray(_vao);
	}
	glBindBuffer(GL_ARRAY_BUFFER, _vbo);
	if (uNumExtraVertices > 0) {
		//need to add 2 vertex copies to create degenerate triangles between this strip
		//and the last strip that was stored in the batch
		glBufferSubData(GL_ARRAY_BUFFER, _uNumUsedVertices * sizeof(GuiVertex), sizeof(GuiVertex), &_lastVertex);
		glBufferSubData(GL_ARRAY_BUFFER, (_uNumUsedVertices + 1) * sizeof(GuiVertex), sizeof(GuiVertex), &vVertices[0]);
	}
	// Use glMapBuffer instead, if moving large chunks of data > 1MB
	glBufferSubData(GL_ARRAY_BUFFER, (_uNumUsedVertices + uNumExtraVertices) * sizeof(GuiVertex), vVertices.size() * sizeof(GuiVertex), &vVertices[0]);
	if (Settings::getOpenglVersion().x >= 3) {
		glBindVertexArray(0);
	}
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	_uNumUsedVertices += vVertices.size() + uNumExtraVertices;
	_lastVertex = vVertices[vVertices.size() - 1];
}//add
//------------------------------------------------------------------------
void Batch::render() {
	if (_uNumUsedVertices == 0) {
		//nothing in this buffer to render
		return;
	}
	bool usingTexture = INVALID_UNSIGNED != _config.uTextureId;
	ShaderManager::setUniform(U_USING_TEXTURE, usingTexture);
	if (usingTexture) {
		ShaderManager::setTexture(0, U_TEXTURE0_SAMPLER_2D, _config.uTextureId);
	}
	ShaderManager::setUniform(U_TRANSFORM_MATRIX, _config.transformMatrix);
	//draw contents of buffer
	if (Settings::getOpenglVersion().x >= 3) {
		glBindVertexArray(_vao);
		glDrawArrays(_config.uRenderType, 0, _uNumUsedVertices);
		glBindVertexArray(0);
	}
	else { //OpenGL v2.x
		glBindBuffer(GL_ARRAY_BUFFER, _vbo);
		unsigned uOffset = 0;
		ShaderManager::enableAttribute(A_POSITION, sizeof(GuiVertex), uOffset);
		uOffset += sizeof(glm::vec2);
		ShaderManager::enableAttribute(A_COLOR, sizeof(GuiVertex), uOffset);
		uOffset += sizeof(glm::vec4);
		ShaderManager::enableAttribute(A_TEXTURE_COORD0, sizeof(GuiVertex), uOffset);
		glDrawArrays(_config.uRenderType, 0, _uNumUsedVertices);
		ShaderManager::disableAttribute(A_POSITION);
		ShaderManager::disableAttribute(A_COLOR);
		ShaderManager::disableAttribute(A_TEXTURE_COORD0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
	//reset buffer
	_uNumUsedVertices = 0;
	_config.iPriority = 0;
}//render





BatchManager::BatchManager(unsigned uNumBatches, unsigned numVerticesPerBatch) :
	_uNumBatches(uNumBatches),
	_maxNumVerticesPerBatch(numVerticesPerBatch)
{
	//test input parameters
	if (uNumBatches < 10) {
		std::ostringstream strStream;
		strStream << __FUNCTION__ << " uNumBatches{" << uNumBatches << "} is too small. Choose a number >= 10 ";
		throw ExceptionHandler(strStream);
	}
	//a good size for each batch is between 1-4MB in size. Number of elements that can be stored in a
	//batch is determined by calculating #bytes used by each vertex
	if (numVerticesPerBatch < 1000) {
		std::ostringstream strStream;
		strStream << __FUNCTION__ << " numVerticesPerBatch{" << numVerticesPerBatch << "} is too small. Choose a number >= 1000 ";
		throw ExceptionHandler(strStream);
	}
	//create desired number of batches
	_vBatches.reserve(uNumBatches);
	for (unsigned u = 0; u < uNumBatches; ++u) {
		_vBatches.push_back(std::shared_ptr(new Batch(numVerticesPerBatch)));
	}
}//BatchManager
//------------------------------------------------------------------------
BatchManager::~BatchManager() {
	_vBatches.clear();
}//~BatchManager
//------------------------------------------------------------------------
void BatchManager::render(const std::vector& vVertices, const BatchConfig& config) {
	Batch* pEmptyBatch = nullptr;
	Batch* pFullestBatch = _vBatches[0].get();
	//determine which batch to put these vertices into
	for (unsigned u = 0; u < _uNumBatches; ++u) {
		Batch* pBatch = _vBatches.get();
		if (pBatch->isBatchConfig(config)) {
			if (!pBatch->isEnoughRoom(vVertices.size())) {
				//first need to empty this batch before adding anything to it
				emptyBatch(false, pBatch);
			}
			pBatch->add(vVertices);
			return;
		}
		//store pointer to first empty batch
		if (nullptr == pEmptyBatch && pBatch->isEmpty()) {
			pEmptyBatch = pBatch;
		}
		//store pointer to fullest batch
		pFullestBatch = pBatch->getFullest(pFullestBatch);
	}
	//if we get here then we didn't find an appropriate batch to put the vertices into
	//if we have an empty batch, put vertices there
	if (nullptr != pEmptyBatch) {
		pEmptyBatch->add(vVertices, config);
		return;
	}
	//no empty batches were found therefore we must empty one first and then we can use it
	emptyBatch(false, pFullestBatch);
	pFullestBatch->add(vVertices, config);
}//render
//------------------------------------------------------------------------
//empty all batches by rendering their contents now
void BatchManager::emptyAll() {
	emptyBatch(true, _vBatches[0].get());
}//emptyAll
//------------------------------------------------------------------------
struct CompareBatch : public std::binary_function {
	bool operator()(const Batch* pBatchA, const Batch* pBatchB) const {
		return (pBatchA->getPriority() > pBatchB->getPriority());
	}//operator()
};//CompareBatch
//------------------------------------------------------------------------
//empties the batches according to priority. If emptyAll is false then
//only empty the batches that are lower priority than the one specified
//AND also empty the one that is passed in
void BatchManager::emptyBatch(bool emptyAll, Batch* pBatchToEmpty) {
	//sort batches by priority
	std::priority_queue, CompareBatch > std::queue;
	for (unsigned u = 0; u < _uNumBatches; ++u) {
		//add all non-empty batches to queue which will be sorted by order
		//from lowest to highest priority
		if (!_vBatches->isEmpty()) {
			if (emptyAll) {
				std::queue.push(_vBatches.get());
			}
			else if (_vBatches->getPriority() < pBatchToEmpty->getPriority()) {
				//only add batches that are lower in priority
				std::queue.push(_vBatches.get());
			}
		}
	}
	//render all desired batches
	while (!std::queue.empty()) {
		Batch* pBatch = std::queue.top();
		pBatch->render();
		std::queue.pop();
	}
	if (!emptyAll) {
		//when not emptying all the batches, we still want to empty
		//the batch that is passed in, in addition to all batches
		//that have lower priority than it
		pBatchToEmpty->render();
	}
}//emptyBatch