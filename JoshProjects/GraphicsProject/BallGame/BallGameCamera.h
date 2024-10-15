#pragma once

// BallGameCamera.h
#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class BallGameCamera {
public:
  BallGameCamera() : _position(0.0f, 0.0f), _scale(1.0f), _needsMatrixUpdate(true) {}

  void init(int screenWidth, int screenHeight) {
    _screenWidth = screenWidth;
    _screenHeight = screenHeight;
    _needsMatrixUpdate = true;
  }

  void update() {
    if (_needsMatrixUpdate) {
      glm::vec3 translate(-_position.x + _screenWidth / 2, -_position.y + _screenHeight / 2, 0.0f);
      _cameraMatrix = glm::translate(glm::mat4(1.0f), translate);

      glm::vec3 scale(_scale, _scale, 1.0f);
      _cameraMatrix = glm::scale(glm::mat4(1.0f), scale) * _cameraMatrix;

      _orthoMatrix = glm::ortho(0.0f, static_cast<float>(_screenWidth), 0.0f, static_cast<float>(_screenHeight));

      _cameraMatrix = _orthoMatrix * _cameraMatrix;

      _needsMatrixUpdate = false;
    }
  }

  glm::vec2 convertScreenToWorld(glm::vec2 screenCoords) const {
    // Invert Y-axis
    screenCoords.y = _screenHeight - screenCoords.y;

    // Make it so that 0 is the center
    screenCoords -= glm::vec2(_screenWidth / 2, _screenHeight / 2);

    // Scale the coordinates
    screenCoords /= _scale;

    // Translate with the camera position
    screenCoords += _position;

    return screenCoords;
  }

  glm::vec2 convertWorldToScreen(glm::vec2 worldCoords) const {
    // Translate with the camera position
    worldCoords -= _position;

    // Scale the coordinates
    worldCoords *= _scale;

    // Make it so that the center is _screenWidth/2, _screenHeight/2
    worldCoords += glm::vec2(_screenWidth / 2, _screenHeight / 2);

    // Invert Y-axis
    worldCoords.y = _screenHeight - worldCoords.y;

    return worldCoords;
  }


  void setPosition(const glm::vec2& newPosition) {
    _position = newPosition;
    _needsMatrixUpdate = true;
  }

  void setScale(float newScale) {
    _scale = newScale;
    _needsMatrixUpdate = true;
  }

  glm::vec2 getPosition() const { return _position; }
  float getScale() const { return _scale; }
  glm::mat4 getCameraMatrix() const { return _cameraMatrix; }

private:
  int _screenWidth, _screenHeight;
  glm::vec2 _position;
  glm::mat4 _cameraMatrix;
  glm::mat4 _orthoMatrix;
  float _scale;
  bool _needsMatrixUpdate;
};
