//Agent.cpp

#include "Agent.h"
#include "Level.h"
#include "JAGEngine/ResourceManager.h"
#include <algorithm>

Agent::Agent() {

}

Agent::~Agent() {

}

void Agent::draw(JAGEngine::SpriteBatch& _spriteBatch) {

  static int textureID = JAGEngine::ResourceManager::getTexture("Textures/zombie_game/spr_npc.png").id;

  const glm::vec4 uvRect(0.0f, 0.0f, 1.0f, 1.0f);

  glm::vec4 destRect;
  destRect.x = _position.x;
  destRect.y = _position.y;
  destRect.z = AGENT_WIDTH;

  destRect.w = AGENT_WIDTH;

  _spriteBatch.draw(destRect, uvRect, textureID, 0.0f, _color);
}

void Agent::update(const std::vector<std::string>& levelData,
  std::vector<Human*>& humans,
  std::vector<Zombie*>& zombies) {

}

void Agent::collideWithLevel(const std::vector<std::string>& levelData) {
  std::vector<glm::vec2> collideTilePositions;

  checkTilePosition(levelData, collideTilePositions, _position.x, _position.y);
  checkTilePosition(levelData, collideTilePositions, _position.x + AGENT_WIDTH, _position.y);
  checkTilePosition(levelData, collideTilePositions, _position.x, _position.y + AGENT_WIDTH);
  checkTilePosition(levelData, collideTilePositions, _position.x + AGENT_WIDTH, _position.y + AGENT_WIDTH);

  for (int i = 0; i < collideTilePositions.size(); i++) {
    collideWithTile(collideTilePositions[i]);
  }
}

void Agent::checkTilePosition(const std::vector<std::string>& levelData,
  std::vector<glm::vec2>& collideTilePositions,
  float x, float y) {
  glm::vec2 cornerPos = glm::vec2(std::floor(x / TILE_WIDTH),
    std::floor(y / TILE_WIDTH));

  if (cornerPos.y >= 0 && cornerPos.y < levelData.size() &&
    cornerPos.x >= 0 && cornerPos.x < levelData[cornerPos.y].size()) {
    if (levelData[cornerPos.y][cornerPos.x] != '.') {
      collideTilePositions.emplace_back(cornerPos.x * TILE_WIDTH + TILE_WIDTH / 2.0f,
        cornerPos.y * TILE_WIDTH + TILE_WIDTH / 2.0f);
    }
  }
}

//AABB collision
void Agent::collideWithTile(glm::vec2 tilePos) {
  const float AGENT_RADIUS = AGENT_WIDTH / 2.0f;
  const float TILE_RADIUS = TILE_WIDTH / 2.0f;
  const float MIN_DISTANCE = AGENT_RADIUS + TILE_RADIUS;

  glm::vec2 centerAgentPos = _position + glm::vec2(AGENT_RADIUS);
  glm::vec2 distVec = centerAgentPos - tilePos;

  float xDepth = MIN_DISTANCE - std::abs(distVec.x);
  float yDepth = MIN_DISTANCE - std::abs(distVec.y);

  if (xDepth > 0 && yDepth > 0) {
    if (xDepth < yDepth) {
      if (distVec.x < 0) {
        _position.x -= xDepth;
      }
      else {
        _position.x += xDepth;
      }
    }
    else {
      if (distVec.y < 0) {
        _position.y -= yDepth;
      }
      else {
        _position.y += yDepth;
      }
    }
  }
}
