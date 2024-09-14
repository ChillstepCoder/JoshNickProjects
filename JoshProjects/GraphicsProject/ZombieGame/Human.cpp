//Human.cpp

#include "Human.h"
#include <random>

Human::Human() {

}

Human::~Human() {

}

void Human::update(const std::vector<std::string>& levelData,
  std::vector<Human*>& humans,
  std::vector<Zombie*>& zombies) {
  

  collideWithLevel(levelData);
}

void Human::init(float speed, glm::vec2 pos) {

  //static std::mt19937 randomEngine;
  //randomEngine
  //static std::uniform_real_distribution<float>(0.01f, 1.0f);

  _speed = speed;
  _position = pos;
  _direction =
}
