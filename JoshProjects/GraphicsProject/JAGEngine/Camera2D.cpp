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

    // Create a larger orthographic projection to handle big world coordinates
    float aspectRatio = (float)screenWidth / (float)screenHeight;
    float orthoWidth = ORTHO_SCALE * screenWidth;
    float orthoHeight = orthoWidth / aspectRatio;

    _orthoMatrix = glm::ortho(
      -orthoWidth / 2.0f,
      orthoWidth / 2.0f,
      -orthoHeight / 2.0f,
      orthoHeight / 2.0f,
      -1.0f,
      1.0f
    );

    // Create separate projection matrix for UI elements
    _projectionMatrix = glm::ortho(
      0.0f,
      (float)screenWidth,
      (float)screenHeight,
      0.0f,
      -1.0f,
      1.0f
    );

    _needsMatrixUpdate = true;
  }

  void Camera2D::update() {
    if (_needsMatrixUpdate) {
      // Normalize position to prevent floating point precision issues
      glm::vec2 normalizedPos = _position;
      if (glm::length(normalizedPos) > MAX_WORLD_SIZE) {
        normalizedPos = glm::normalize(normalizedPos) * MAX_WORLD_SIZE;
      }

      // Create translation matrix
      glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f),
        glm::vec3(-normalizedPos.x, -normalizedPos.y, 0.0f));

      // Create scale matrix
      glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f),
        glm::vec3(_scale, _scale, 1.0f));

      // Combine matrices in correct order
      _cameraMatrix = _orthoMatrix * scaleMatrix * translationMatrix;

      _needsMatrixUpdate = false;
    }
  }

  glm::vec2 Camera2D::convertScreenToWorld(glm::vec2 screenCoords) {
    // Convert screen coordinates to normalized coordinates
    glm::vec2 normalizedCoords;
    normalizedCoords.x = (screenCoords.x / static_cast<float>(_screenWidth)) * 2.0f - 1.0f;
    normalizedCoords.y = 1.0f - (screenCoords.y / static_cast<float>(_screenHeight)) * 2.0f;

    // Convert to world coordinates
    glm::vec4 clipCoords(normalizedCoords.x, normalizedCoords.y, 0.0f, 1.0f);
    glm::vec4 worldCoords = glm::inverse(_cameraMatrix) * clipCoords;

    return glm::vec2(worldCoords.x, worldCoords.y);
  }

  bool Camera2D::isBoxInView(const glm::vec2& position, const glm::vec2& dimensions) {
    // Scale view bounds based on current zoom
    glm::vec2 scaledDimensions = dimensions / _scale;
    glm::vec2 viewBounds = glm::vec2(_screenWidth, _screenHeight) * (ORTHO_SCALE / _scale);

    // Calculate distances
    glm::vec2 centerPos = position + dimensions * 0.5f;
    glm::vec2 distVec = centerPos - _position;

    // Check if box is within view bounds
    bool xInView = std::abs(distVec.x) < (viewBounds.x * 0.5f + scaledDimensions.x * 0.5f);
    bool yInView = std::abs(distVec.y) < (viewBounds.y * 0.5f + scaledDimensions.y * 0.5f);

    return xInView && yInView;
  }
}
