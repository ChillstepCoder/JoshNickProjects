//GameSystem.h

#pragma once
#include "Player.h"
#include "Level.h"
#include "Enemy.h"

class GameSystem
{
public:
  GameSystem();
  GameSystem(int levelnum);

  void tickGame();
  void playGame();
  void playerMove();
  void changeLevel(int newLevel, int playerX, int playerY);
  void enterShop();
  void gameOver();

private:
  Level _level;
  Player _player;
  bool _playerInShop = false;
  static const int GRID_SIZE = 3;
  int getLevelAbove(int currentLevel) const;
  int getLevelBelow(int currentLevel) const;
  int getLevelLeft(int currentLevel) const;
  int getLevelRight(int currentLevel) const;
  bool _levelChanged;
  bool _needsRedraw = false;
};
