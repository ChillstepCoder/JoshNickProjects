//Camera2D.cpp

#include "Camera2D.h"

namespace JAGEngine {

  Camera2D::Camera2D() : _position(0.0f, 0.0f),
    _cameraMatrix(1.0f),
    _orthoMatrix(1.0f),
    _scale(1.0f),
    _needsMatrixUpdate(true),
    _screenWidth(1000),
    _screenHeight(1000)
  {

  }

  Camera2D::~Camera2D()
  {

  }

  void Camera2D::init(int screenWidth, int screenHeight) {
    _screenWidth = screenWidth;
    _screenHeight = screenHeight;
    _orthoMatrix = glm::ortho(0.0f, static_cast<float>(_screenWidth), 0.0f, static_cast<float>(_screenHeight));
  }


  void Camera2D::update() {
    if (_needsMatrixUpdate) {
      // Start with the identity matrix
      _cameraMatrix = glm::mat4(1.0f);

      // Scale around the center of the screen
      glm::vec3 scale(_scale, _scale, 1.0f);
      glm::vec3 centerScreen(_screenWidth / 2.0f, _screenHeight / 2.0f, 0.0f);
      _cameraMatrix = glm::translate(_cameraMatrix, centerScreen);
      _cameraMatrix = glm::scale(_cameraMatrix, scale);
      _cameraMatrix = glm::translate(_cameraMatrix, -centerScreen);

      // Translate the camera
      glm::vec3 translate(-_position.x, -_position.y, 0.0f);
      _cameraMatrix = glm::translate(_cameraMatrix, translate);

      // Apply orthographic projection
      _cameraMatrix = _orthoMatrix * _cameraMatrix;

      _needsMatrixUpdate = false;
    }
  }

  glm::vec2 Camera2D::convertScreenToWorld(glm::vec2 screenCoords) {

    //invert y direction
    screenCoords.y = _screenHeight - screenCoords.y;
    //make 0 the center
    screenCoords -= glm::vec2(_screenWidth / 2, _screenHeight / 2);
    //scale the coordinates
    screenCoords /= _scale;
    //translate with camera position
    screenCoords += _position;

    return screenCoords;
  }


}
