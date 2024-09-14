//Human.cpp

#include "Human.h"
#include <random>
#include <ctime>

#define GLM_ENABLE_EXPERIMENTAL

#include <glm/gtx/rotate_vector.hpp>

Human::Human() :
  _frames(0)
{

}

Human::~Human() {

}

void Human::update(const std::vector<std::string>& levelData,
  std::vector<Human*>& humans,
  std::vector<Zombie*>& zombies) {

  const int UPDATE_CAP = 120;

  static std::mt19937 randomEngine(time(nullptr));
  
  static std::uniform_int_distribution<int> randCap(1, UPDATE_CAP-1);
  static std::uniform_real_distribution<float> randRotate(-45.0f, 45.0f);
  
  _position += _direction * _speed;
  int cap = UPDATE_CAP;
  if (_frames == UPDATE_CAP) {
    cap = randCap(randomEngine);
    _direction = glm::rotate(_direction, randRotate(randomEngine) * ((float)cap/ (float)UPDATE_CAP));
    _frames = cap;
  }
  else {
    _frames++;
  }

  collideWithLevel(levelData);
}

void Human::init(float speed, glm::vec2 pos) {



  static std::mt19937 randomEngine(time(nullptr));
  static std::uniform_real_distribution<float> randDir(-1.0f, 1.0f);

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
}
