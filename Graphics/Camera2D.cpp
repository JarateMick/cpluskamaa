#include "Camera2D.h"

namespace UpiEngine {

	Camera2D::Camera2D() :
		_needsMatrixUpdate(true),
		_position(0.0f, 0.0f),
		_cameraMatrix(1.0f),
		_orthoMatrix(1.0f),
		_scale(1.0f),
		_screenWidth(500),
		_screenHeight(500)
	{
	}


	Camera2D::~Camera2D()
	{
	}

	void Camera2D::init(int screenWidth, int screenHeight)
	{
		_screenWidth = screenWidth;
		_screenHeight = screenHeight;
		_orthoMatrix = glm::ortho(0.0f, (float)_screenWidth, 0.0f, (float)_screenHeight);
	}

	void Camera2D::update()
	{
		if (_needsMatrixUpdate)
		{
			//glm::vec3 translate(-_position.x, -_position.y, 0.0f);
			//negatiivinen position,että kun menemme ite vasemmalle kaikki muu menee oikealle  kolmas olisi kolmiulotteiselle
			//translate liikuttaa objecteja ruudussa 

			glm::vec3 translate(-_position.x + _screenWidth / 2, -_position.y + _screenHeight / 2, 0.0f);  //centeroi cameran myösä
			_cameraMatrix = glm::translate(_orthoMatrix, translate);


			// kun yksi niin normaali eli kerrotaan yhdellä | jos 0.5 zoomattu ulos ja 2.o niiin zoom in
			glm::vec3 scale(_scale, _scale, 0.0f);
			_cameraMatrix = glm::scale(glm::mat4(1.0f), scale) * _cameraMatrix;
			//camera scale^^
			        

			_needsMatrixUpdate = false;
		}
	}

	glm::vec2 Camera2D::convertScreenToWorld(glm::vec2 screenCoords)
	{
		//invert y direction
		screenCoords.y = _screenHeight - screenCoords.y;
		// tehdään keskustasta 0,0
		screenCoords -= glm::vec2(_screenWidth / 2, _screenHeight / 2);
		//scalettaa maailman koordinaatiti
		screenCoords /= _scale;
		//translate with the camera position
		screenCoords += _position;
		return screenCoords;
	}
	//AABB TEST to see if the box is in the camera view
	bool Camera2D::isBoxinView(const glm::vec2 &position, const glm::vec2 &dimensions)
	{
		glm::vec2 scaledScreenDimensions = glm::vec2(_screenWidth, _screenHeight) / _scale ;

		const float MIN_DISTANCE_X = dimensions.x / 2.0f + scaledScreenDimensions.x / 2.0f;
		const float MIN_DISTANCE_Y = dimensions.y / 2.0f + scaledScreenDimensions.y / 2.0f;


		glm::vec2 centerPos = position + dimensions / 2.0f;
		
		glm::vec2 centerCameraPos = _position;		//center position of the camera
		
		glm::vec2 distVec = centerPos - centerCameraPos;			//vector from the input to the camera

		float xDepth = MIN_DISTANCE_X - abs(distVec.x);
		float yDepth = MIN_DISTANCE_Y - abs(distVec.y);

		if (xDepth > 0 && yDepth > 0)
		{

				// there was a collision 
				return true;
			
		}
		return false;
	}
}