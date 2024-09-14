//Level.cpp

#include "Level.h"
#include <JAGEngine/Errors.h>
#include <fstream>
#include <JAGEngine/ResourceManager.h>

Level::Level(const std::string& fileName) {


  std::ifstream file;
  file.open(fileName);


  //error checking
  if (file.fail()) {
    JAGEngine::fatalError("Failed to open " + fileName);
  }


  //throw away 1st string in tmp
  std::string tmp;
  file >> tmp >> _numHumans;

  std::getline(file, tmp); //throw away the rest of the 1st line

  //read the level data;
  while (std::getline(file, tmp)) {
    _levelData.push_back(tmp);
  }
  _spriteBatch.init();
  _spriteBatch.begin();

  glm::vec4 uvRect(0.0f, 0.0f, 1.0f, 1.0f);
  JAGEngine::Color whiteColor;
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
        _spriteBatch.draw(destRect,
          uvRect,
          JAGEngine::ResourceManager::getTexture("Textures/zombie_game/spr_stone.png").id,
          0.0f,
          whiteColor);
        break;
      case 'B':
        _spriteBatch.draw(destRect,
          uvRect,
          JAGEngine::ResourceManager::getTexture("Textures/zombie_game/spr_brick.png").id,
          0.0f,
          whiteColor);
        break;
      case 'W':
        _spriteBatch.draw(destRect,
          uvRect,
          JAGEngine::ResourceManager::getTexture("Textures/zombie_game/spr_wood.png").id,
          0.0f,
          whiteColor);
        break;
      case 'Z':
        _levelData[y][x] = '.';
        _zombieStartPositions.emplace_back(x * TILE_WIDTH, y * TILE_WIDTH);
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

    _spriteBatch.end();
}

Level::~Level() {

}


void Level::draw() {
  _spriteBatch.renderBatch();
}
