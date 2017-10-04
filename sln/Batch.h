#pragma once
#include <glm/mat4x2.hpp>

class Batch;
struct BatchConfig;
class BatchManager sealed{
public:
private:
	std::vector > _vBatches;
	unsigned _uNumBatches;
	unsigned _maxNumVerticesPerBatch;
	//^^^^------ variables above ------|------ functions below ------vvvv
public:
	BatchManager(unsigned uNumBatches, unsigned numVerticesPerBatch);
	~BatchManager();
	void render(const std::vector& vVertices, const BatchConfig& config);
	void emptyAll();
protected:
private:
	BatchManager(const BatchManager& c); //not implemented
	BatchManager& operator=(const BatchManager& c); //not implemented
	void emptyBatch(bool emptyAll, Batch* pBatchToEmpty);
};//BatchManager

struct GuiVertex {
	glm::vec2 position;
	glm::vec4 color;
	glm::vec2 texture;
	GuiVertex(glm::vec2 positionIn, glm::vec4 colorIn, glm::vec2 textureIn = glm::vec2()) :
		position(positionIn),
		color(colorIn),
		texture(textureIn)
	{}
};//GuiVertex

struct BatchConfig {
	unsigned uRenderType;
	int iPriority;
	unsigned uTextureId;
	glm::mat4 transformMatrix; //initialized as identity matrix
	BatchConfig(unsigned uRenderTypeIn, int iPriorityIn, unsigned uTextureIdIn) :
		uRenderType(uRenderTypeIn),
		iPriority(iPriorityIn),
		uTextureId(uTextureIdIn)
	{}
	bool operator==(const BatchConfig& other) const {
		if (uRenderType != other.uRenderType ||
			iPriority != other.iPriority ||
			uTextureId != other.uTextureId ||
			transformMatrix != other.transformMatrix)
		{
			return false;
		}
		return true;
	}
	bool operator!=(const BatchConfig& other) const {
		return !(*this == other);
	}
};//BatchConfig

class Batch sealed {
public:
private:
	unsigned	_uMaxNumVertices;
	unsigned	_uNumUsedVertices;
	unsigned	_vao; //only used in OpenGL v3.x +
	unsigned	_vbo;
	BatchConfig _config;
	GuiVertex _lastVertex;
	//^^^^------ variables above ------|------ functions below ------vvvv
public:
	Batch(unsigned uMaxNumVertices);
	~Batch();
	bool isBatchConfig(const BatchConfig& config) const;
	bool isEmpty() const;
	bool isEnoughRoom(unsigned uNumVertices) const;
	Batch* getFullest(Batch* pBatch);
	int getPriority() const;
	void add(const std::vector& vVertices, const BatchConfig& config);
	void add(const std::vector& vVertices);
	void render();
protected:
private:
	Batch(const Batch& c); //not implemented
	Batch& operator=(const Batch& c); //not implemented
	void cleanUp();

};//Batch