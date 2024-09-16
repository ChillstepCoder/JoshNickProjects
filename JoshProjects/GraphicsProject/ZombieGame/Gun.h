//Gun.h

#pragma once
#include <string>
#include <glm/glm.hpp>
#include <vector>
#include "Bullet.h"

class Gun
{
public:
  Gun(std::string name, int fireRate, int bulletsperShot, float spread, float bulletDamage, float bulletSpeed);
  ~Gun();


  void update(bool isMouseDown, const glm::vec2& position, const glm::vec2& direction, std::vector<Bullet>& bullets);

  

private:
  void fire(const glm::vec2& direction, const glm::vec2& position, std::vector<Bullet>& bullets);

  std::string _name;

  int _fireRate;

  int _bulletsPerShot;

  float _spread;

  float _bulletSpeed;

  float _bulletDamage;

  int _frameCounter;

};

