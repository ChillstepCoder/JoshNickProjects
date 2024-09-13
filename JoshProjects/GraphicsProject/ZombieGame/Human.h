//Human.h

#pragma once
#include "Agent.h"

class Human : public Agent
{
public:
  Human();
  virtual ~Human();
  virtual void update(const std::vector<std::string>& levelData,
    std::vector<Human*>& humans,
    std::vector<Zombie*>& zombies) = 0;
};
