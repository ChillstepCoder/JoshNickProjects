#include "Bullet.h"
#include "Agent.h"
#include "Human.h"
#include "Zombie.h"
#include "Level.h"

#include <JAGEngine/ResourceManager.h>
Bullet::Bullet(glm::vec2 position, glm::vec2 direction, float damage, float speed) :
  _position(position),
  _direction(direction),
  _damage(damage),
  _speed(speed)
{

}

Bullet::~Bullet() {

}

bool Bullet::update(const std::vector<std::string>& levelData, float deltaTime) {
  _position += _direction * _speed * deltaTime;
  return collideWithWorld(levelData);
}

void Bullet::draw(JAGEngine::SpriteBatch& spriteBatch) {
  glm::vec4 destRect(_position.x - BULLET_RADIUS, _position.y - BULLET_RADIUS, BULLET_RADIUS * 2, BULLET_RADIUS * 2);
  const glm::vec4 uvRect(0.0f, 0.0f, 1.0f, 1.0f);
  JAGEngine::ColorRGBA8 color;
  color.r = 200;
  color.g = 200;
  color.b = 200;
  color.a = 255;
  spriteBatch.draw(destRect, uvRect, JAGEngine::ResourceManager::getTexture("Textures/zombie_game/bullet.png").id, 0.0f, color);
}

bool Bullet::collideWithWorld(const std::vector<std::string>& levelData) {
  glm::ivec2 gridPosition;
  gridPosition.x = floor(_position.x / (float)TILE_WIDTH);
  gridPosition.y = floor(_position.y / (float)TILE_WIDTH);

  // Check if the bullet is out of bounds
  if (gridPosition.y < 0 || gridPosition.y >= levelData.size() ||
    gridPosition.x < 0 || gridPosition.x >= levelData[0].size()) {
    return true; // Collided (out of bounds)
  }

  // Check if the bullet hit a wall
  return (levelData[gridPosition.y][gridPosition.x] != '.');
}

bool Bullet::collideWithAgent(Agent* agent) {

  const float MIN_DISTANCE = AGENT_RADIUS + BULLET_RADIUS;

  glm::vec2 centerPosA = _position;
  glm::vec2 centerPosB = agent->getPosition();

  glm::vec2 distVec = centerPosA - centerPosB;

  float distance = glm::length(distVec);

  float collisionDepth = MIN_DISTANCE - distance;

  if (collisionDepth > 0) {
    return true;
  }
  else {
    return false;
  }
}
