#pragma once
#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>

namespace UpiEngine {

	class Camera2D
	{
	public:
		Camera2D();
		~Camera2D();

		//laittaa orthographic matrixsin ja ruutu dimensionit kohdilleen
		void init(int screenWidth, int screenHeight);

		void update(); //updatee kamera matrixin jos on tarvetta

		glm::vec2 convertScreenToWorld(glm::vec2 screenCoords);

		bool isBoxinView(const glm::vec2 &position, const glm::vec2 &dimensions);

		//setters
		void setPosition(const glm::vec2 &newPosition) { _position = newPosition; _needsMatrixUpdate = true; }
		void setScale(float newScale) { _scale = newScale; _needsMatrixUpdate = true; }

		//getters
		float getScale() { return _scale; }
		glm::vec2 getPosition() { return _position; }
		glm::mat4 getCameraMatrix() { return _cameraMatrix; }
	private:
		int _screenWidth, _screenHeight;
		bool _needsMatrixUpdate;
		float _scale;
		glm::vec2 _position;  //sama kuin vec2 shader fragmentissa kiitos glm
		glm::mat4 _cameraMatrix;
		glm::mat4 _orthoMatrix;



	};
}