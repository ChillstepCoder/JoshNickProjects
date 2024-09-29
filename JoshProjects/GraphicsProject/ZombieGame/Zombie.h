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
    std::vector<Zombie*>& zombies,
    float deltaTime) override;

private:
  Human* getNearestHuman(std::vector<Human*>& humans);
  bool hasLineOfSight(const glm::vec2& start, const glm::vec2& end, const std::vector<std::string>& levelData);
  int _frames;                         
  int _nextUpdateFrame;
};
