#include "Player.h"
#include <SDL/SDL.h>

#include "Gun.h"

Player::Player() : _currentGunIndex(-1) {
  // Empty
}

Player::~Player() {
  //empty
}

void Player::init(float speed, glm::vec2 pos, JAGEngine::InputManager* inputManager, JAGEngine::Camera2D* camera, std::vector<Bullet>* bullets) {
  _speed = speed;
  _position = pos;
  _health = 200;
  _color.r = 255;
  _color.g = 255;
  _color.b = 255;
  _color.a = 255;
  _inputManager = inputManager;
  _camera = camera;
  _bullets = bullets;
}

void Player::addGun(Gun* gun) {
  //add gun to inventory
  _guns.push_back(gun);
  if (_currentGunIndex == -1) {
    _currentGunIndex = 0;
  }
}

void Player::update(const std::vector<std::string>& levelData,
  std::vector<Human*>& humans,
  std::vector<Zombie*>& zombies) {
  if (_inputManager->isKeyPressed(SDLK_w)) {
    _position.y += _speed;
  }
  else if (_inputManager->isKeyPressed(SDLK_s)) {
    _position.y -= _speed;
  }
  if (_inputManager->isKeyPressed(SDLK_a)) {
    _position.x -= _speed;
  }
  else if (_inputManager->isKeyPressed(SDLK_d)) {
    _position.x += _speed;
  }

  if (_inputManager->isKeyPressed(SDLK_1) && _guns.size() >= 1) {
    _currentGunIndex = 0;
  }
  if (_inputManager->isKeyPressed(SDLK_2) && _guns.size() >= 2) {
    _currentGunIndex = 1;
  }
  if (_inputManager->isKeyPressed(SDLK_3) && _guns.size() >= 3) {
    _currentGunIndex = 2;
  }

  if (_currentGunIndex != -1 && _bullets != nullptr) {
    glm::vec2 mouseCoords = _inputManager->getMouseCoords();
    mouseCoords = _camera->convertScreenToWorld(mouseCoords);
    glm::vec2 centerPosition = _position;
    glm::vec2 direction = glm::normalize(mouseCoords - centerPosition);
    _guns[_currentGunIndex]->update(_inputManager->isKeyPressed(SDL_BUTTON_LEFT),
      centerPosition,
      direction,
      *_bullets);
  }

  collideWithLevel(levelData);
}
