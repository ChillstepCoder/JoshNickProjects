#include "GameSystem.h"
#include "Level.h"
#include "Soldier.h"
#include <fstream>
#include <iostream> 
#include <cstdio>
#include <conio.h>
#include <string>
#include <vector>
#include <random>
#include <chrono>
#include <assert.h>

Level::Level() : _currentLevel(1), gameSystem(nullptr) {
}

Level::SoldierCounts Level::getInitialTeam1SoldierCounts() const {
  return _initialTeam1Counts;
}

bool Level::load(int num) {
  _levelData.clear();
  _soldierGrid.clear();
  _levelGrid.clear();
  for (int i = 0; i < NUM_ARMIES; i++) {
    _armies[i].clear();
  }

  std::ifstream inputFile;
  std::string originalFilename = "Level" + std::to_string(num) + ".txt";
  inputFile.open(originalFilename);
  if (!inputFile) {
    perror(originalFilename.c_str());
    return false;
  }

  std::string input;
  while (std::getline(inputFile, input)) {
    _levelData.push_back(input + "\n");
    std::vector<Soldier*> newRow(input.size(), nullptr);
    _soldierGrid.push_back(newRow);
  }
  inputFile.close();

  char tile;
  _initialTeam1Counts = SoldierCounts();
  for (int i = 0; i < _levelData.size(); i++) {
    for (int ii = 0; ii < _levelData[i].size(); ii++) {
      tile = _levelData[i][ii];
      switch (tile) {
        //TEAM 1 (PLAYER)
      case 'S':
        _armies[0].push_back(new Soldier("Soldier", tile, 1, 10, 10, 20, 0, 100, 1, true));
        _armies[0].back()->setPosition(ii, i);
        _soldierGrid[i][ii] = _armies[0].back();
        _initialTeam1Counts.soldiers++;
        break;
      case 'P':
        _armies[0].push_back(new Soldier("Paladin", tile, 1, 18, 18, 40, 0, 100, 1, true));
        _armies[0].back()->setPosition(ii, i);
        _soldierGrid[i][ii] = _armies[0].back();
        _initialTeam1Counts.paladins++;
        break;
        //TEAM 2 (ENEMY)
      case 'D':
        _armies[1].push_back(new Soldier("Dragon", tile, 5, 20, 20, 40, 1, 100, 1, true));
        _armies[1].back()->setPosition(ii, i);
        _soldierGrid[i][ii] = _armies[1].back();
        break;
      case 'g':
        _armies[1].push_back(new Soldier("Goblin", tile, 1, 5, 5, 10, 1, 100, 1, true));
        _armies[1].back()->setPosition(ii, i);
        _soldierGrid[i][ii] = _armies[1].back();
        break;
      case 'o':
        _armies[1].push_back(new Soldier("Orc", tile, 1, 8, 8, 15, 1, 100, 1, true));
        _armies[1].back()->setPosition(ii, i);
        _soldierGrid[i][ii] = _armies[1].back();
        break;
      case 'O':
        _armies[1].push_back(new Soldier("Ogre", tile, 1, 16, 16, 25, 1, 100, 1, true));
        _armies[1].back()->setPosition(ii, i);
        _soldierGrid[i][ii] = _armies[1].back();
        break;
      }
    }
  }
  _currentLevel = num;
  shuffleArmies();
  return true;
}

void Level::shuffleArmies() {
  std::random_device::result_type seed = std::random_device()();
  std::mt19937 randomEngine(seed);
  int rand;
  Soldier* tmp;

  //shuffle armies
  for (int i = 0; i < NUM_ARMIES; i++) {
    for (int j = _armies[i].size() - 1; j > 1; j--) {
      std::uniform_int_distribution<int> Roll(0, j - 1);
      rand = Roll(randomEngine);
      tmp = _armies[i][j];
      _armies[i][j] = _armies[i][rand];
      _armies[i][rand] = tmp;
    }
  }
}

char Level::getTile(int x, int y) const {
  if (y >= 0 && y < _levelData.size() && x >= 0 && x < _levelData[y].size()) {
    return _levelData[y][x];
  }
  return '#';
}

Soldier* Level::getSoldier(int x, int y) const {
  return _soldierGrid[y][x];
}

std::string Level::generateLevel(int seed) {
  _levelData.clear();
  for (int i = 0; i < levelHeight; i++) {
    std::string row;
    for (int ii = 0; ii < levelWidth; ii++) {
      if (i == 0 || i == (levelHeight - 1) || ii == 0 || ii == (levelWidth - 1)) {
        row += "#";
      }
      else {
        row += ".";
      }
    }
    row += "\n";
    _levelData.push_back(row);
  }
  std::string levelString;
  for (int i = 0; i < _levelData.size(); i++) {
    levelString += _levelData[i];
  }
  return levelString;
}


void Level::printVoid() const {
  system("cls");
}

void Level::printStats() const {

}

void Level::printStats2() const {

}

void Level::print() const {

  std::string levelString;
  levelString.reserve(_levelData.size() * (_levelData[0].size() + 1));

  for (int y = 0; y < _levelData.size(); y++) {
    for (int x = 0; x < _levelData[y].size(); x++) {
      levelString += _levelData[y][x];
    }
  }

  printf("%s", levelString.c_str());
}
void Level::setTile(int x, int y, char tile, Soldier* soldier) {
  if (y >= 0 && y < _levelData.size() && x >= 0 && x < _levelData[y].size()) {
    _levelData[y][x] = tile;
    _soldierGrid[y][x] = soldier;
  }
}

void Level::setGameSystem(GameSystem* gs) {
  gameSystem = gs;
}

bool Level::isGameOver() const {
  return _armies[0].empty();
}

bool Level::isVictory() const {
  for (int i = 1; i < NUM_ARMIES; i++) {
    if (!_armies[i].empty()) {
      return false;
    }
  }
  return true;
}

void Level::update() {
  bool anyMoved;
  int updateCount = 0;
  do {
    anyMoved = false;
    updateCount++;

    for (int j = 0; j < NUM_ARMIES; j++) {
      for (size_t i = 0; i < _armies[j].size(); i++) {
        updateOpenMoves(_armies[j][i]);
        char move = _armies[j][i]->getMove(_armies, NUM_ARMIES);
        if (move != '.') {
          processSoldierMove(move, _armies[j][i]);
          anyMoved = true;
        }
      }
    }

    // Remove dead soldiers and update the board
    for (int j = 0; j < NUM_ARMIES; j++) {
      for (size_t i = 0; i < _armies[j].size(); ) {
        if (_armies[j][i]->getHealth() <= 0) {
          int x, y;
          _armies[j][i]->getPosition(x, y);
          setTile(x, y, '.', nullptr);
          delete _armies[j][i];
          _armies[j][i] = _armies[j].back();
          _armies[j].pop_back();
        }
        else {
          i++;
        }
      }
    }

    // Update the visual representation
    for (int y = 0; y < levelHeight; y++) {
      for (int x = 0; x < levelWidth; x++) {
        Soldier* soldier = getSoldier(x, y);
        if (soldier) {
          setTile(x, y, soldier->getTile(), soldier);
        }
      }
    }
    printVoid();
    print();
    if (!isVictory()) {
      gameSystem->printStats();
      _getch();
    }
  } while (anyMoved);

  //check for gameover or victory
  if (isGameOver()) {
    gameSystem->gameOver();
  } else if (isVictory()) {
    gameSystem->levelComplete();
  }
}

void Level::processSoldierMove(char direction, Soldier *soldier) {
  int x, y;
  int targetX, targetY;
  soldier->getPosition(x, y);

  switch (direction) {
  case 'w': //up
    targetX = x;
    targetY = y - 1;
    break;
  case 'a': //left
    targetX = x - 1;
    targetY = y;
    break;
  case 's': //down
    targetX = x;
    targetY = y + 1;
    break;
  case 'd': //right
    targetX = x + 1;
    targetY = y;
    break;
  case '.':
    return;
  }

  char targetTile = getTile(targetX, targetY);

  switch (targetTile) {
  case '#':
    break;
  case '.':
    moveSoldier(soldier, targetX, targetY);
    break;
  default:
    battle(soldier, targetX, targetY);
    break;
  }
}

void Level::battle(Soldier* soldier, int targetX, int targetY) {
  int x, y;
  int enemyArmy;
  soldier->getPosition(x, y);

  Soldier* targetSoldier = getSoldier(targetX, targetY);

  if (targetSoldier == nullptr) {
    return;
  }

  enemyArmy = targetSoldier->getTeam();
  if (enemyArmy == soldier->getTeam()) {
    return;
  }

  int result = targetSoldier->takeDamage(soldier->attack());
  if (result == 1) {
    for (int i = 0; i < _armies[enemyArmy].size(); i++) {
      if (_armies[enemyArmy][i] == targetSoldier) {
        _armies[enemyArmy][i] = _armies[enemyArmy].back();
        _armies[enemyArmy].pop_back();
        setTile(targetX, targetY, '.', nullptr);
        break;
      }
    }
  }
}

void Level::moveSoldier(Soldier* soldier, int targetX, int targetY) {
  int x, y;
  soldier->getPosition(x, y);

  setTile(x, y, '.', nullptr);
  setTile(targetX, targetY, soldier->getTile(), soldier);

  soldier->setPosition(targetX, targetY);
}

void Level::updateOpenMoves(Soldier* soldier) {
  int x, y;
  soldier->getPosition(x, y);

  bool leftPossible = (x > 0) && (getTile(x - 1, y) == '.' || (getSoldier(x - 1, y) && getSoldier(x - 1, y)->getTeam() != soldier->getTeam()));
  bool rightPossible = (x < levelWidth - 1) && (getTile(x + 1, y) == '.' || (getSoldier(x + 1, y) && getSoldier(x + 1, y)->getTeam() != soldier->getTeam()));
  bool upPossible = (y > 0) && (getTile(x, y - 1) == '.' || (getSoldier(x, y - 1) && getSoldier(x, y - 1)->getTeam() != soldier->getTeam()));
  bool downPossible = (y < levelHeight - 1) && (getTile(x, y + 1) == '.' || (getSoldier(x, y + 1) && getSoldier(x, y + 1)->getTeam() != soldier->getTeam()));

  soldier->setMoves(leftPossible, rightPossible, upPossible, downPossible);
}

int Level::getSoldierCount(int armyIndex) const {
  return (_armies[armyIndex].size());
}

Level::SoldierCounts Level::countSoldiers(const std::vector<Soldier*>& army) const {
  SoldierCounts counts;
  for (int i = 0; i < army.size(); i++) {
    if (army[i]->getName() == "Soldier") {
      counts.soldiers++;
    }
    else if (army[i]->getName() == "Paladin") {
      counts.paladins++;
    }
  }
  return counts;
}

Level::SoldierCounts Level::getTeam1SoldierCounts() const {
  return countSoldiers(_armies[0]);//returns struct of various unit counts
}
