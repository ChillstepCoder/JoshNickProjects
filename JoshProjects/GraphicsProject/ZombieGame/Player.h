//Player.h

#pragma once
#include "Human.h"
#include <JAGEngine/InputManager.h>

class Player : public Human
{
public:
  Player();
  ~Player();
  void init(float speed, glm::vec2 pos, JAGEngine::InputManager* inputManager);
  void update(const std::vector<std::string>& levelData,
    std::vector<Human*>& humans,
    std::vector<Zombie*>& zombies) override;
private:
  JAGEngine::InputManager* _inputManager;
};
