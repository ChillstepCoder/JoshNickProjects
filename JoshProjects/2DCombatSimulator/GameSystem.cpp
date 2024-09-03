#include "GameSystem.h"
#include <iostream>
#include <conio.h>
#include "Level.h"


GameSystem::GameSystem() : _level(), _currentLevel(1), _levelChanged(false) {
  _level.setGameSystem(this);
}

GameSystem::GameSystem(int levelnum) : _level(), _currentLevel(levelnum), _levelChanged(false) {
  _level.load(levelnum);
  _level.print();
  printStats();
}

void GameSystem::playGame() {
  while (true) {
    if (!_level.load(_currentLevel)) {
      if (!_level.isGameOver() && !_level.isVictory()) {
        std::cout << "Game completed! Congratulations!" << std::endl;
        std::cout << "Press any key to start a new game..." << std::endl;
        _getch();
        _currentLevel = 1;
      }
      continue;
    }
    _level.print();
    printStats();
    _getch();
    bool levelCompleted = false;
    while (!levelCompleted) {
      _getch();
      _level.update();
      _level.printVoid();
      _level.print();
      printStats();
      
      if (_level.isGameOver() || _level.isVictory()) {
        levelCompleted = true;
      }
    }

    if (_level.isVictory()) {
      levelComplete();
      levelCompleted = false;
    } else {
      gameOver();
      break;
    }
  }
}

void GameSystem::printStats() {
  Level::SoldierCounts currentTeam1Counts = _level.getTeam1SoldierCounts();
  int soldierTotals = currentTeam1Counts.soldiers + currentTeam1Counts.paladins;
  if (soldierTotals > 0) {

    std::cout << "\nYou have:" << std::endl;
  }
  if (currentTeam1Counts.soldiers > 0) {
    std::cout << currentTeam1Counts.soldiers << " Soldier" << (currentTeam1Counts.soldiers != 1 ? "s" : "") << std::endl;
  }
  if (currentTeam1Counts.paladins > 0) {
    std::cout << currentTeam1Counts.paladins << " Paladin" << (currentTeam1Counts.paladins != 1 ? "s" : "") << std::endl;
  }
}

void GameSystem::changeLevel(int newLevel) {
  _level.setCurrentLevel(newLevel);
  _level.load(newLevel);
  _levelChanged = true;
  _level.printVoid();
  _level.print();
  printStats();
}

void GameSystem::gameOver() {
  std::cout << "Game Over! Your army has been defeated." << std::endl;
  std::cout << "Press enter to restart the game..." << std::endl;
  while (true) {
    int ch = _getch();
    if (ch == '\r' || ch == '\n') {
      break;
    }
  }
  _currentLevel = 1;
  changeLevel(_currentLevel);
}

void GameSystem::levelComplete() {
  std::cout << "Victory! You have defeated the enemy army." << std::endl;

  Level::SoldierCounts initialTeam1Counts = _level.getInitialTeam1SoldierCounts();
  Level::SoldierCounts currentTeam1Counts = _level.getTeam1SoldierCounts();

  int soldierLosses = initialTeam1Counts.soldiers - currentTeam1Counts.soldiers;
  int paladinLosses = initialTeam1Counts.paladins - currentTeam1Counts.paladins;

  std::cout << "\nYou have lost:" << std::endl;
  if (initialTeam1Counts.soldiers > 0) {
    std::cout << soldierLosses << " Soldier" << (soldierLosses != 1 ? "s" : "") << std::endl;
  }
  if (initialTeam1Counts.paladins > 0) {
    std::cout << paladinLosses << " Paladin" << (paladinLosses != 1 ? "s" : "") << std::endl;
  }

  std::cout << "\nYou now have:" << std::endl;
  if (initialTeam1Counts.soldiers > 0) {
    std::cout << currentTeam1Counts.soldiers << " Soldier" << (currentTeam1Counts.soldiers != 1 ? "s" : "") << std::endl;
  }
  if (initialTeam1Counts.paladins > 0) {
    std::cout << currentTeam1Counts.paladins << " Paladin" << (currentTeam1Counts.paladins != 1 ? "s" : "") << std::endl;
  }

  std::cout << "\nPress enter to continue to the next level..." << std::endl;
  while (true) {
    int ch = _getch();
    if (ch == '\r' || ch == '\n') {
      break;
    }
  }

  _currentLevel++;
  changeLevel(_currentLevel);
}

void GameSystem::enterShop() {
  _playerInShop = true;
  _playerInShop = false;
}
