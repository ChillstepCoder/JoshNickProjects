//GameSystem.cpp

#include "GameSystem.h"
#include "SaveManager.h"
#include <iostream>
#include <conio.h>
#include "Level.h"
#include "Player.h"


GameSystem::GameSystem() : _level(), _player() {
  _player.init(1, 10, 1, 1, 0, 5, 1); // Level, HP, ATK, DEF, EXP, XPTO, LUCK
  _levelChanged = false;
  _level.setPlayer(&_player);
  _level.setGameSystem(this);
}

GameSystem::GameSystem(int levelnum) : _level(), _player() {
  _levelChanged = false;
  _player.init(1, 10, 1, 1, 0, 5, 1); // Level, HP, ATK, DEF, EXP, XPTO, LUCK
  _level.setPlayer(&_player);
  _level.load(levelnum, _player);
  _level.print();
}

void GameSystem::playerMove() {
  if (_playerInShop) return;
  char input = _getch();
  int playerX, playerY;
  _player.getPosition(playerX, playerY);
  int newX = playerX;
  int newY = playerY;

  switch (input) {
  case 'w': case 'W': newY--; break;
  case 's': case 'S': newY++; break;
  case 'a': case 'A': newX--; break;
  case 'd': case 'D': newX++; break;
  case 'f':
  case 'F':
    _player.useFood();
    _level.updateEnemies(_player);
    return;
  case 'p':
  case 'P':
    _player.usePotion();
    _level.updateEnemies(_player);
    return;
  default: return;
  }

  int currentLevel = _level.getCurrentLevel();
  int newLevel = currentLevel;

  if (newX < 0 || newX >= _level.getWidth() || newY < 0 || newY >= _level.getHeight()) {
    // Handle level change
    if (newX < 0) newLevel = getLevelLeft(currentLevel);
    if (newX >= _level.getWidth()) newLevel = getLevelRight(currentLevel);
    if (newY < 0) newLevel = getLevelAbove(currentLevel);
    if (newY >= _level.getHeight()) newLevel = getLevelBelow(currentLevel);

    if (newLevel != currentLevel) {
      changeLevel(newLevel, (newX + _level.getWidth()) % _level.getWidth(),
        (newY + _level.getHeight()) % _level.getHeight());
    }
  }
  else {
    _level.processPlayerMove(_player, newX, newY);
  }
  _needsRedraw = true;
}

void GameSystem::playGame() {

  int levelNumber = 1;
  if (SaveManager::saveExists()) {
    SaveManager::loadGame(_player, levelNumber);
  }
  else {
    _player.init(1, 10, 1, 1, 0, 5, 1);// Level, HP, ATK, DEF, EXP, XPTO, LUCK
  }

  if (!_level.load(levelNumber, _player)) {
    std::cout << "Failed to load level " << levelNumber << std::endl;
    return;
  }
  bool gamePlaying = true;
  bool needsRedraw = true;

  while (gamePlaying) {
    if (!_playerInShop) {
      playerMove();
      _level.updateEnemies(_player);
    }
    if (needsRedraw) {
      _level.printVoid();
      _level.printStats();
      _level.print();
      _level.printStats2();
      needsRedraw = false;
    }

    if (_levelChanged) {
      SaveManager::saveGame(_player, _level.getCurrentLevel());
      _levelChanged = false;
    }
    needsRedraw = true;
  }
}

void GameSystem::changeLevel(int newLevel, int playerX, int playerY) {
  _level.save(_level.getCurrentLevel());
  _level.setCurrentLevel(newLevel);
  _level.load(newLevel, _player);
  _player.setPosition(playerX, playerY);
  _levelChanged = true;
}


int GameSystem::getLevelAbove(int currentLevel) const {
  if (currentLevel > GRID_SIZE) return currentLevel - GRID_SIZE;
  return currentLevel;
}

int GameSystem::getLevelBelow(int currentLevel) const {
  if (currentLevel <= GRID_SIZE * (GRID_SIZE - 1)) return currentLevel + GRID_SIZE;
  return currentLevel;
}

int GameSystem::getLevelLeft(int currentLevel) const {
  if (currentLevel % GRID_SIZE != 1) return currentLevel - 1;
  return currentLevel;
}

int GameSystem::getLevelRight(int currentLevel) const {
  if (currentLevel % GRID_SIZE != 0) return currentLevel + 1;
  return currentLevel;
}

void GameSystem::gameOver() {
  std::cout << "Game Over!" << std::endl;
  SaveManager::clearSaves();
  _player.init(1, 10, 1, 1, 0, 5, 1); // Level, HP, ATK, DEF, EXP, XPTO, LUCK
  _level.setCurrentLevel(1);
  std::cout << "Press any key to start a new game..." << std::endl;
  _getch();
  playGame();
}

void GameSystem::enterShop() {
  _playerInShop = true;
  _level.interactWithShop(_player);
  _playerInShop = false;
}
