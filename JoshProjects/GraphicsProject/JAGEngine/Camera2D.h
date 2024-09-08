//Camera2D.h

#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace JAGEngine {
  class Camera2D
  {
  public:
    Camera2D();
    ~Camera2D();

    void init(int screenWidth, int screenHeight);
    void update();

    //setters
    void setPosition(const glm::vec2& newPosition) { _position = newPosition; _needsMatrixUpdate = true; }
    void setScale(float newScale) { _scale = newScale; _needsMatrixUpdate = true; }

    //getters
    glm::vec2 getPosition() { return _position; }
    float getScale() const { return _scale; }
    glm::mat4 getOrthoMatrix() { return _orthoMatrix; }
    glm::mat4 getCameraMatrix() { return _cameraMatrix; }

  private:
    int _screenWidth, _screenHeight;
    bool _needsMatrixUpdate;
    float _scale;
    glm::vec2 _position;
    glm::mat4 _cameraMatrix;
    glm::mat4 _orthoMatrix;
  };

}
