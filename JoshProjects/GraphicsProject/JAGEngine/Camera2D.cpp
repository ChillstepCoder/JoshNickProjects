//Camera2D.cpp

#include "Camera2D.h"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

namespace JAGEngine {

  Camera2D::Camera2D() :
    _position(0.0f, 0.0f),
    _cameraMatrix(1.0f),
    _orthoMatrix(1.0f),
    _scale(1.0f),
    _needsMatrixUpdate(true),
    _screenWidth(1000),
    _screenHeight(1000) {
  }

  Camera2D::~Camera2D() {
  }

  void Camera2D::init(int screenWidth, int screenHeight) {
    _screenWidth = screenWidth;
    _screenHeight = screenHeight;
    _orthoMatrix = glm::ortho(
      -static_cast<float>(screenWidth) / 2.0f,
      static_cast<float>(screenWidth) / 2.0f,
      -static_cast<float>(screenHeight) / 2.0f,
      static_cast<float>(screenHeight) / 2.0f,
      -1.0f,
      1.0f
    );
    _needsMatrixUpdate = true;
  }


  void Camera2D::update() {
    if (_needsMatrixUpdate) {
      // Start with identity matrix
      _cameraMatrix = glm::mat4(1.0f);

      // Apply scale transformation first
      _cameraMatrix = glm::scale(_cameraMatrix, glm::vec3(_scale, _scale, 1.0f));

      // Then apply translation
      _cameraMatrix = glm::translate(_cameraMatrix, glm::vec3(-_position.x, -_position.y, 0.0f));

      // Combine with orthographic projection
      _cameraMatrix = _orthoMatrix * _cameraMatrix;

      _needsMatrixUpdate = false;

      // Debug output
      std::cout << "Camera updated - Position: (" << _position.x << ", " << _position.y
        << ") Scale: " << _scale << "\n";
    }
  }

  glm::vec2 Camera2D::convertScreenToWorld(glm::vec2 screenCoords) {
    // Step 1: Convert screen coordinates to normalized device coordinates (-1 to 1)
    screenCoords.x = (screenCoords.x / static_cast<float>(_screenWidth)) * 2.0f - 1.0f;
    screenCoords.y = 1.0f - (screenCoords.y / static_cast<float>(_screenHeight)) * 2.0f;

    // Step 2: Create 4D vector for transformation
    glm::vec4 screenPos(screenCoords.x, screenCoords.y, 0.0f, 1.0f);

    // Step 3: Transform by inverse camera matrix
    glm::vec4 worldPos = glm::inverse(_cameraMatrix) * screenPos;

    // Debug output
    std::cout << "Screen to World conversion:\n"
      << "  Screen: (" << screenCoords.x << ", " << screenCoords.y << ")\n"
      << "  World: (" << worldPos.x << ", " << worldPos.y << ")\n";

    return glm::vec2(worldPos.x, worldPos.y);
  }

  //AABB test to see if box is in the camera view
  bool Camera2D::isBoxInView(const glm::vec2& position, const glm::vec2& dimensions) {
    glm::vec2 scaledScreenDimensions = glm::vec2(_screenWidth, _screenHeight) / _scale;

    const float MIN_DISTANCE_X = dimensions.x / 2.0f + scaledScreenDimensions.x / 2.0f;
    const float MIN_DISTANCE_Y = dimensions.y / 2.0f + scaledScreenDimensions.y / 2.0f;

    glm::vec2 centerPos = position + dimensions / 2.0f;
    glm::vec2 centerCameraPos = _position;
    glm::vec2 distVec = centerPos - centerCameraPos;

    float xDepth = MIN_DISTANCE_X - std::abs(distVec.x);
    float yDepth = MIN_DISTANCE_Y - std::abs(distVec.y);

    return (xDepth > 0 && yDepth > 0);
  }

}
