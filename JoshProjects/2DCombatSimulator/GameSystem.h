#pragma once

#include "Level.h"

class GameSystem
{
public:
  GameSystem();
  GameSystem(int levelnum);
  void playGame();
  void printStats();
  void changeLevel(int newLevel);
  void enterShop();
  void gameOver();
  void levelComplete();

private:
  Level _level;
  int _currentLevel;
  bool _playerInShop;
  bool _levelChanged;
  bool _needsRedraw;
};

