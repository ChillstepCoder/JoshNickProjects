//Level.h

#pragma once
#include <vector>
#include <string>
#include "Player.h"
#include "Enemy.h"

class GameSystem;

class Level
{
public:

  Level();
  bool load(int num, Player& player);
  bool save(int num = 0);
  std::string generateLevel(int seed = 0);
  void print();
  void printStats();
  void printStats2();
  void printVoid();
  void movePlayer(char input, Player& player);
  void updateEnemies(Player& player);
  void interactWithShop(Player& player);
  void interactWithFood(Player& player, int x, int y);
  void interactWithKey(Player& player, int x, int y);
  void interactWithDoor(Player& player, int x, int y);

  //getters
  char getTile(int x, int y) const;
  int getCurrentLevel() const { return _currentLevel; }
  //pair<int, int> getPlayerPosition() const { return _playerPosition; }
  int getHeight() const { return levelHeight; }
  int getWidth() const { return levelWidth; }
  int getEnemyCount() const { return static_cast<int>(_enemies.size()); }

  //setters
  void setTile(int x, int y, char tile);
  void setCurrentLevel(int level) { _currentLevel = level; }
  void setPlayerPosition(int x, int y) { _playerPosition = { x, y }; }
  void setPlayer(Player* player) { _player = player; }
  void placePlayer(const Player& player);
  void clearPlayer();
  void processPlayerMove(Player& player, int targetX, int targetY);
  void processEnemyMove(Player& player, int enemyIndex, int targetX, int targetY);
  void setGameSystem(GameSystem* gs);

private:
  Player* _player;
  GameSystem* gameSystem;
  void battleMonster(Player &player, int targetX, int targetY);
  bool _enemiesCanMove = true;
  std::vector<std::string> _levelData;
  std::vector<int> _levelGrid;
  std::vector<Enemy> _enemies;

  int _currentLevel;
  std::pair<int, int> _playerPosition;

  const int levelWidth = 80;
  const int levelHeight = 25;
};
