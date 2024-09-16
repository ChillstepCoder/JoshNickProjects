//Zombie.h

#pragma once

#include "Agent.h"

const float CHASE_DISTANCE = 400.0f;

class Zombie : public Agent
{
public:
  Zombie();
  ~Zombie();

  void init(float speed, glm::vec2 pos);

  virtual void update(const std::vector<std::string>& levelData,
    std::vector<Human*>& humans,
    std::vector<Zombie*>& zombies);

private:
  Human* getNearestHuman(std::vector<Human*>& humans);
  glm::vec2 _direction;
  int _frames;
};
