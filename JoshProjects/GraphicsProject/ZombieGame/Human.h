//Human.h

#pragma once
#include "Agent.h"

const float FLEE_DISTANCE = 250.0f;

class Human : public Agent
{
public:
  Human();
  virtual ~Human();
  void init(float speed, glm::vec2 pos);
  virtual void update(const std::vector<std::string>& levelData,
    std::vector<Human*>& humans,
    std::vector<Zombie*>& zombies,
    float deltaTime) override;

  // Add these public methods
  float getZombify() const { return _zombify; }
  void incrementZombify(float amount) { _zombify += amount; }

private:
  Zombie* getNearestZombie(std::vector<Zombie*>& zombies);
  int _frames;
  int _nextUpdateFrame;
  float _zombify = 0.0f;
};
