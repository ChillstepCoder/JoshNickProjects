//Zombie.h

#pragma once

#include "Agent.h"

class Zombie : public Agent
{
public:
  Zombie();
  ~Zombie();

  virtual void update(const std::vector<std::string>& levelData,
    std::vector<Human*>& humans,
    std::vector<Zombie*>& zombies) = 0;
};
