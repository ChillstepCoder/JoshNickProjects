//Human.h

#pragma once
#include "Agent.h"

const float FLEE_DISTANCE = 150.0f;

class Human : public Agent
{
public:
  Human();
  virtual ~Human();

  void init(float speed, glm::vec2 pos);

  virtual void update(const std::vector<std::string>& levelData,
    std::vector<Human*>& humans,
    std::vector<Zombie*>& zombies);

private:
  Zombie* getNearestZombie(std::vector<Zombie*>& zombies);
  glm::vec2 _direction;
  int _frames;
};
