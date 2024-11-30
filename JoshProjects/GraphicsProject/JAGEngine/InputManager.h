// InputManager.h

#pragma once
#include <unordered_map>
#include <glm/glm.hpp>

namespace JAGEngine {

  class InputManager
  {
  public:
    InputManager();
    ~InputManager();

    void update();

    void pressKey(unsigned int keyID);
    void releaseKey(unsigned int keyID);

    //setters
    void setMouseCoords(float x, float y);

    //getters
    bool isKeyDown(unsigned int keyID);
    bool isKeyPressed(unsigned int keyID);

    //getters
    glm::vec2 getMouseCoords() const { return _mouseCoords; }


  private:
    bool wasKeyDown(unsigned int keyID);

    std::unordered_map<unsigned int, bool> _keyMap;
    std::unordered_map<unsigned int, bool> _previousKeyMap;
    glm::vec2 _mouseCoords;
  };

}
