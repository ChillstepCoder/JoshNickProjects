#include "InputManager.h"
#include <iostream>

namespace JAGEngine {

  InputManager::InputManager() : _mouseCoords(0.0f) {

  }

  InputManager::~InputManager() {

  }

  // InputManager.cpp
  void InputManager::pressKey(unsigned int keyID) {
    std::cout << "Key pressed: " << keyID << std::endl;  // Debug print
    _keyMap[keyID] = true;
  }

  void InputManager::releaseKey(unsigned int keyID) {
    std::cout << "Key released: " << keyID << std::endl;  // Debug print
    _keyMap[keyID] = false;
  }

  void InputManager::setMouseCoords(float x, float y) {
    _mouseCoords.x = x;
    _mouseCoords.y = y;
  }

  bool InputManager::isKeyDown(unsigned int keyID) {
    auto it = _keyMap.find(keyID);
    bool isDown = (it != _keyMap.end() && it->second);
    if (isDown) {
      std::cout << "Key " << keyID << " is down\n";  // Debug print
    }
    return isDown;
  }

  bool InputManager::isKeyPressed(unsigned int keyID) {

    if (isKeyDown(keyID) == true && wasKeyDown(keyID) == false) {
      return true;
    }
    return false;
  }

  void InputManager::update() {
    //loop through key map and copy to previous key map
    for (auto& it : _keyMap) {
      _previousKeyMap[it.first] = it.second;
    }
  }

  bool InputManager::wasKeyDown(unsigned int keyID) {
    auto it = _previousKeyMap.find(keyID);
    if (it != _previousKeyMap.end()) {
      return it->second;
    }
    else {
      return false;
    }
  }

}
