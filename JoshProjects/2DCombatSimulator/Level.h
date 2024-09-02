#ifndef LEVEL_H
#define LEVEL_H

#include <vector>
#include <string>
#include "Soldier.h"

class GameSystem;
const int NUM_ARMIES = 2;

class Level
{
public:
  struct SoldierCounts {
    int soldiers = 0;
    int paladins = 0;
  };

  Level();
  bool load(int num);
  std::string generateLevel(int seed = 0);
  void print() const;
  void printStats() const;
  void printStats2() const;
  void printVoid() const;
  bool isGameOver() const;
  bool isVictory() const;
  void update();
  void updateOpenMoves(Soldier* soldier);

  // getters
  char getTile(int x, int y) const;
  Soldier* getSoldier(int x, int y) const;
  int getCurrentLevel() const { return _currentLevel; }
  int getHeight() const { return levelHeight; }
  int getWidth() const { return levelWidth; }
  int getSoldierCount(int armyIndex) const;
  SoldierCounts getTeam1SoldierCounts() const;
  SoldierCounts getInitialTeam1SoldierCounts() const;
  // setters
  void setTile(int x, int y, char tile, Soldier* soldier);
  void setCurrentLevel(int level) { _currentLevel = level; }
  void setGameSystem(GameSystem* gs);

private:
  void processSoldierMove(char direction, Soldier* soldier);
  void battle(Soldier* soldier, int targetX, int targetY);
  void moveSoldier(Soldier* soldier, int targetX, int targetY);
  void shuffleArmies();
  GameSystem* gameSystem;
  bool _armiesCanMove = true;
  std::vector<std::string> _levelData;
  std::vector<std::vector <Soldier*> > _soldierGrid;
  std::vector<int> _levelGrid;
  std::vector<Soldier*> _armies[NUM_ARMIES];
  int _currentLevel = 1;
  const int levelWidth = 80;
  const int levelHeight = 25;
  SoldierCounts countSoldiers(const std::vector<Soldier*>& army) const;
  SoldierCounts _initialTeam1Counts;
};

#endif // LEVEL_H
