//Gun.h

#pragma once
#include <string>
#include <glm/glm.hpp>
#include <vector>
#include "Bullet.h"
#include <JAGEngine/AudioEngine.h>

class Gun
{
public:
  Gun(std::string name, int fireRate, int bulletsperShot,
      float spread, float bulletDamage, float bulletSpeed, JAGEngine::SoundEffect fireEffect);
  ~Gun();


  void update(bool isMouseDown, const glm::vec2& position, const glm::vec2& direction, std::vector<Bullet>& bullets, float deltaTime);

  

private:

  JAGEngine::SoundEffect m_fireEffect;

  void fire(const glm::vec2& direction, const glm::vec2& position, std::vector<Bullet>& bullets);

  std::string _name;

  int _fireRate;

  int m_bulletsPerShot;

  float _spread;

  float m_bulletspeed;

  float _bulletDamage;

  float _frameCounter;

};

