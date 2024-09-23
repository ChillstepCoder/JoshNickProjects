//Player.h
#pragma once
#include "Human.h"
#include <JAGEngine/InputManager.h>
#include <JAGEngine/Camera2D.h>
#include "Bullet.h"
#include <vector>

class Gun;

class Player : public Human
{
public:
  Player();
  ~Player();
  void init(float speed, glm::vec2 pos, JAGEngine::InputManager* inputManager, JAGEngine::Camera2D* camera, std::vector<Bullet>* bullets);
  void addGun(Gun* gun);
  void update(const std::vector<std::string>& levelData,
    std::vector<Human*>& humans,
    std::vector<Zombie*>& zombies,
    float deltaTime) override;

private:
  JAGEngine::InputManager* m_inputManager;
  JAGEngine::Camera2D* m_camera;
  std::vector<Gun*> _guns;
  int _currentGunIndex;
  std::vector<Bullet>* m_bullets;
};
