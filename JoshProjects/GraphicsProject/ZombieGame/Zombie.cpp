//Zombie.cpp

#include "Zombie.h"
#include "Human.h"
#include <random>
#include <ctime>

#define GLM_ENABLE_EXPERIMENTAL

#include <glm/gtx/rotate_vector.hpp>

Zombie::Zombie() :
  _frames(0),
  _direction(1.0f, 0.0f) {  // Initialize direction
}

Zombie::~Zombie() {
}

void Zombie::init(float speed, glm::vec2 pos) {
  _speed = speed;
  _position = pos;
  _health = 100;
  _color = JAGEngine::ColorRGBA8(0,255,0,255);
}

void Zombie::update(const std::vector<std::string>& levelData,
  std::vector<Human*>& humans,
  std::vector<Zombie*>& zombies,
  float deltaTime) {
  const int UPDATE_CAP = 120;
  float closestDistance = 99999.0f;
  static std::mt19937 randomEngine(time(nullptr));
  static std::uniform_int_distribution<int> randCap(1, UPDATE_CAP - 1);
  static std::uniform_real_distribution<float> randRotate(-45.0f, 45.0f);

  Human* closestHuman = getNearestHuman(humans);

  // Calculate the distance to the closest human if one exists
  if (closestHuman != nullptr) {
    glm::vec2 humanPosition = closestHuman->getPosition();
    closestDistance = glm::distance(humanPosition, _position);
  }

  if (closestHuman != nullptr && closestDistance < CHASE_DISTANCE) {
    glm::vec2 direction = glm::normalize(closestHuman->getPosition() - _position);
    _position += direction * _speed * deltaTime;
  }
  else {
    // random movement
    _position += _direction * _speed * deltaTime;

    if (_frames >= UPDATE_CAP) {
      int cap = randCap(randomEngine);
      float angle = glm::radians(randRotate(randomEngine));
      _direction = glm::rotate(_direction, angle);
      _frames = 0;
    }
    else {
      _frames++;
    }
  }

  collideWithLevel(levelData);
}

Human* Zombie::getNearestHuman(std::vector<Human*>& humans) {
  Human* closestHuman = nullptr;
  float smallestDistance = 9999999;
  for (int i = 0; i < humans.size(); i++) {
    glm::vec2 distVec = humans[i]->getPosition() - _position;
    float distance = glm::length(distVec);
    if (distance < smallestDistance) {
      smallestDistance = distance;
      closestHuman = humans[i];
    }
  }
  return closestHuman;
}
