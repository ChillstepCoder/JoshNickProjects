//Zombie.cpp

#include "Zombie.h"
#include "Human.h"
#include <random>
#include <ctime>
#include <SDL/SDL.h>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <JAGEngine/ResourceManager.h>

#define GLM_ENABLE_EXPERIMENTAL

#include <glm/gtx/rotate_vector.hpp>

const float TILE_WIDTH = 64.0f;

Zombie::Zombie() :
  _frames(0),
  _direction(1.0f, 0.0f),
  _nextUpdateFrame(0) {  // Initialize direction
}

Zombie::~Zombie() {
}

void Zombie::init(float speed, glm::vec2 pos) {
  _speed = speed;
  _position = pos;
  _health = 100;
  _color = JAGEngine::ColorRGBA8(0,255,0,255);
  m_textureID = JAGEngine::ResourceManager::getTexture("Textures/zombie_game/spr_npc.png").id;
}

void Zombie::update(const std::vector<std::string>& levelData,
  std::vector<Human*>& humans,
  std::vector<Zombie*>& zombies,
  float deltaTime) {
  const int UPDATE_CAP = 120;
  float closestDistance = 99999.0f;
  static std::mt19937 randomEngine(time(nullptr));
  static std::uniform_real_distribution<float> randRotate(-45.0f, 45.0f);
  Human* closestHuman = getNearestHuman(humans);

  // Calculate the distance to the closest human if one exists
  if (closestHuman != nullptr) {
    glm::vec2 humanPosition = closestHuman->getPosition();
    closestDistance = glm::distance(humanPosition, _position);
  }

  if (closestHuman != nullptr && closestDistance < CHASE_DISTANCE) {
    glm::vec2 direction = glm::normalize(closestHuman->getPosition() - _position);
    _position += direction * _speed * deltaTime;
  }
  else {
    // random movement
    _position += _direction * _speed * deltaTime;

    _frames++;
    if (_frames >= _nextUpdateFrame) {
      // Calculate rotation based on time since last update
      float rotationFactor = static_cast<float>(_frames) / static_cast<float>(UPDATE_CAP);
      float rotation = randRotate(randomEngine) * rotationFactor;

      _direction = glm::rotate(_direction, glm::radians(rotation));

      // Reset frames and set new random update interval
      _frames = 0;
      _nextUpdateFrame = std::uniform_int_distribution<int>(1, UPDATE_CAP - 1)(randomEngine);
    }
  }

  collideWithLevel(levelData);
}

Human* Zombie::getNearestHuman(std::vector<Human*>& humans) {
  Human* closestHuman = nullptr;
  float smallestDistance = 9999999;
  for (int i = 0; i < humans.size(); i++) {
    glm::vec2 distVec = humans[i]->getPosition() - _position;
    float distance = glm::length(distVec);
    if (distance < smallestDistance) {
      smallestDistance = distance;
      closestHuman = humans[i];
    }
  }
  return closestHuman;
}

//Human* Zombie::getNearestHuman(std::vector<Human*>& humans, const std::vector<std::string>& levelData) {
//  Human* closestHuman = nullptr;
//  float smallestDistance = 9999999.0f;
//
//  for (int i = 0; i < humans.size(); i++) {
//    glm::vec2 distVec = humans[i]->getPosition() - _position;
//    float distance = glm::length(distVec);
//
//    if (distance < smallestDistance) {
//      // Check if there's a clear line of sight
//      if (hasLineOfSight(_position, humans[i]->getPosition(), levelData)) {
//        smallestDistance = distance;
//        closestHuman = humans[i];
//      }
//    }
//  }
//
//  return closestHuman;
//}

bool Zombie::hasLineOfSight(const glm::vec2& start, const glm::vec2& end, const std::vector<std::string>& levelData) {
  glm::vec2 dir = end - start;
  float distance = glm::length(dir);
  dir = glm::normalize(dir);

  for (float i = 0; i < distance; i += 1.0f) {  // Check every 1 unit
    glm::vec2 checkPos = start + dir * i;
    int tileX = static_cast<int>(checkPos.x / TILE_WIDTH);
    int tileY = static_cast<int>(checkPos.y / TILE_WIDTH);

    // Check if we're out of bounds
    if (tileX < 0 || tileX >= levelData[0].size() || tileY < 0 || tileY >= levelData.size()) {
      return false;
    }

    // Check if there's a wall at this position
    if (levelData[tileY][tileX] != '.') {
      return false;
    }
  }

  return true;
}
