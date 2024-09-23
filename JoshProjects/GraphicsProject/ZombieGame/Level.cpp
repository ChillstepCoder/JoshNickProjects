#include "Level.h"
#include <JAGEngine/JAGErrors.h>
#include <fstream>
#include <JAGEngine/ResourceManager.h>
#include <iostream>

Level::Level(const std::string& fileName) {
  std::ifstream file;
  file.open(fileName);
  if (file.fail()) {
    throw std::runtime_error("Failed to open " + fileName);
  }
  //throw away 1st string in tmp
  std::string tmp;
  file >> tmp >> _numHumans;
  std::getline(file, tmp); //throw away the rest of the 1st line
  //read the level data;
  while (std::getline(file, tmp)) {
    _levelData.push_back(tmp);
  }

  std::cout << "Level data loaded. Width: " << _levelData[0].size() << ", Height: " << _levelData.size() << std::endl;

  m_spriteBatch.init();
  m_spriteBatch.begin();
  glm::vec4 uvRect(0.0f, 0.0f, 1.0f, 1.0f);
  JAGEngine::ColorRGBA8 whiteColor;
  whiteColor.r = 255;
  whiteColor.g = 255;
  whiteColor.b = 255;
  whiteColor.a = 255;
  //render all the tiles
  for (int y = 0; y < _levelData.size(); y++) {
    for (int x = 0; x < _levelData[y].size(); x++) {
      char tile = _levelData[y][x];
      //get destination rect
      glm::vec4 destRect(x * TILE_WIDTH, y * TILE_WIDTH, TILE_WIDTH, TILE_WIDTH);
      switch (tile) {
      case '#':
      {
        auto texture = JAGEngine::ResourceManager::getTexture("Textures/zombie_game/spr_stone.png");
        if (texture.id == 0) {
          std::cout << "Failed to load texture: Textures/zombie_game/spr_stone.png" << std::endl;
        }
        else {
          m_spriteBatch.draw(destRect, uvRect, texture.id, 0.0f, whiteColor);
        }
      }
      break;
      case 'B':
      {
        auto texture = JAGEngine::ResourceManager::getTexture("Textures/zombie_game/spr_brick.png");
        if (texture.id == 0) {
          std::cout << "Failed to load texture: Textures/zombie_game/spr_brick.png" << std::endl;
        }
        else {
          m_spriteBatch.draw(destRect, uvRect, texture.id, 0.0f, whiteColor);
        }
      }
      break;
      case 'W':
      {
        auto texture = JAGEngine::ResourceManager::getTexture("Textures/zombie_game/spr_wood.png");
        if (texture.id == 0) {
          std::cout << "Failed to load texture: Textures/zombie_game/spr_wood.png" << std::endl;
        }
        else {
          m_spriteBatch.draw(destRect, uvRect, texture.id, 0.0f, whiteColor);
        }
      }
      break;
      case 'Z':
        _levelData[y][x] = '.';
        m_zombiestartPositions.emplace_back(x * TILE_WIDTH, y * TILE_WIDTH);
        break;
      case '@':
        _levelData[y][x] = '.';
        _startPlayerPos.x = x * TILE_WIDTH;
        _startPlayerPos.y = y * TILE_WIDTH;
        break;
      case '.':
        break;
      default:
        std::printf("Unexpected Symbol %c at (%d,%d)", tile, x, y);
        break;
      }
    }
  }
  m_spriteBatch.end();

  std::cout << "Level initialization complete. Zombies: " << m_zombiestartPositions.size()
    << ", Player start: (" << _startPlayerPos.x << ", " << _startPlayerPos.y << ")" << std::endl;
}

Level::~Level() {
}

void Level::draw() {
  m_spriteBatch.renderBatch();
}

int Level::getWidth() const {
  if (_levelData.empty()) {
    std::cerr << "Error: Level data is empty!" << std::endl;
    return 0;
  }
  return _levelData[0].size();
}
