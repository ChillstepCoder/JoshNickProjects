// Camera2D.h

#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace JAGEngine {
  class Camera2D {
  public:
    Camera2D();
    ~Camera2D();
    void init(int screenWidth, int screenHeight);
    void update();
    glm::vec2 convertScreenToWorld(glm::vec2 screenCoords);
    bool isBoxInView(const glm::vec2& position, const glm::vec2& dimensions);

    // Setters
    void setPosition(const glm::vec2& newPosition) { _position = newPosition; _needsMatrixUpdate = true; }
    void setScale(float newScale) { _scale = newScale; _needsMatrixUpdate = true; }

    // Getters
    glm::vec2 getPosition() const { return _position; }
    float getScale() const { return _scale; }
    glm::mat4 getCameraMatrix() const { return _cameraMatrix; }
    glm::mat4 getOrthoMatrix() const { return _orthoMatrix; }

  private:
    static constexpr float MAX_WORLD_SIZE = 1000000.0f;  // Maximum world coordinates
    static constexpr float ORTHO_SCALE = 2.0f;  // Scale factor for orthographic projection

    static constexpr float WORLD_SCALE = 100.0f;

    int _screenWidth;
    int _screenHeight;
    bool _needsMatrixUpdate;
    float _scale;
    glm::vec2 _position;
    glm::mat4 _cameraMatrix;
    glm::mat4 _orthoMatrix;
    glm::mat4 _projectionMatrix;
  };
}
