//Human.cpp

#include "Human.h"
#include "Zombie.h"
#include <random>
#include <ctime>
#include <SDL/SDL.h>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <JAGEngine/ResourceManager.h>

#define GLM_ENABLE_EXPERIMENTAL

#include <glm/gtx/rotate_vector.hpp>

Human::Human() :
  _frames(0),
  _nextUpdateFrame(0)
{

}

Human::~Human() {

}

void Human::update(const std::vector<std::string>& levelData,
  std::vector<Human*>& humans,
  std::vector<Zombie*>& zombies,
  float deltaTime) {
  const int UPDATE_CAP = 60;
  Zombie* closestZombie = getNearestZombie(zombies);
  float closestDistance = 99999.0f;
  static std::mt19937 randomEngine(time(nullptr));
  static std::uniform_real_distribution<float> randRotate(-90.0f, 90.0f);

  // Calculate the distance to the closest zombie if one exists
  if (closestZombie != nullptr) {
    glm::vec2 zombiePosition = closestZombie->getPosition();
    closestDistance = glm::distance(zombiePosition, _position);
  }

  // flee
  if (closestZombie != nullptr && closestDistance < FLEE_DISTANCE) {
    glm::vec2 direction = glm::normalize(_position - closestZombie->getPosition());
    _position += direction * _speed * deltaTime;
  }
  else {
    // random movement
    _position += _direction * _speed * deltaTime;

    _frames++;
    if (_frames >= UPDATE_CAP) {
      // Calculate rotation based on time since last update
      float rotationFactor = static_cast<float>(_frames) / static_cast<float>(UPDATE_CAP);
      float rotation = randRotate(randomEngine) * rotationFactor;

      _direction = glm::rotate(_direction, glm::radians(rotation));

      // Reset frames and set new random update interval
      _frames = 0;
      _nextUpdateFrame = std::uniform_int_distribution<int>(1, UPDATE_CAP - 1)(randomEngine);
    }
  }

  collideWithLevel(levelData);
}

void Human::init(float speed, glm::vec2 pos) {

  static std::mt19937 randomEngine(time(nullptr));
  static std::uniform_real_distribution<float> randDir(-1.0f, 1.0f);

  _health = 20;

  _color.r = 200;
  _color.g = 0;
  _color.b = 200;
  _color.a = 255;

  _speed = speed;
  _position = pos;
  // get random direction
  _direction = glm::vec2(randDir(randomEngine), randDir(randomEngine));
  //make sure direction isnt 0
  if (_direction.length() == 0) _direction = glm::vec2(1.0f, 0.0f);
  
  _direction = glm::normalize(_direction);
  m_textureID = JAGEngine::ResourceManager::getTexture("Textures/zombie_game/spr_npc.png").id;
}


Zombie* Human::getNearestZombie(std::vector<Zombie*>& zombies) {
  Zombie* closestZombie = nullptr;
  float smallestDistance = 99999;

  for (int i = 0; i < zombies.size(); i++) {
    glm::vec2 distVec = zombies[i]->getPosition() - _position;
    float distance = glm::length(distVec);

    if (distance < smallestDistance) {
      smallestDistance = distance;
      closestZombie = zombies[i];
    }
  }

  return closestZombie;
}
