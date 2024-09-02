// SaveManager.cpp
#include "SaveManager.h"
#include <fstream>
#include <io.h>
#include <direct.h>
#include "Shop.h"

using namespace std;

const string SaveManager::SAVE_FILE_PREFIX = "SaveLevel";
const string SaveManager::PLAYER_SAVE_FILE = "PlayerSave.txt";

void SaveManager::saveGame(const Player& player, int currentLevel) {
  ofstream playerFile(PLAYER_SAVE_FILE);
  playerFile << player.getLevel() << " " << player.getHealth() << " " << player.getMaxHealth() << " "
    << player.getAttack() << " " << player.getDefense() << " "
    << player.getExperience() << " " << player.getXpToNextLevel() << " "
    << player.getGold() << " " << player.getKeys() << " "
    << player.getLuck() << " "
    << currentLevel << "\n";
  int x, y;
  player.getPosition(x, y);
  playerFile << x << " " << y << "\n";

  // Save inventory
  const int* inventory = player.getInventory();
  for (int i = 0; i < Shop::numItems; ++i) {
    playerFile << inventory[i] << " ";
  }
  playerFile << "\n";

  playerFile.close();

  // Save level data
  string levelFileName = SAVE_FILE_PREFIX + to_string(currentLevel) + ".txt";
  ifstream sourceLevel("Level" + to_string(currentLevel) + ".txt");
  ofstream saveLevel(levelFileName);
  saveLevel << sourceLevel.rdbuf();
  sourceLevel.close();
  saveLevel.close();
}

bool SaveManager::loadGame(Player& player, int& currentLevel) {
  ifstream playerFile(PLAYER_SAVE_FILE);
  int level, health, maxHealth, attack, defense, experience, xpToNext, gold, keys, luck, x, y;
  playerFile >> level >> health >> maxHealth >> attack >> defense >> experience >> xpToNext
    >> gold >> keys >> luck >> currentLevel;
  playerFile >> x >> y;

  player.init(level, maxHealth, attack, defense, experience, xpToNext, luck);
  player.setHealth(health);
  player.setPosition(x, y);
  player.addGold(gold);
  for (int i = 0; i < keys; ++i) {
    player.addKey();
  }

  // Load inventory
  int itemCount;
  for (int i = 0; i < Shop::numItems; ++i) {
    playerFile >> itemCount;
    for (int j = 0; j < itemCount; ++j) {
      player.addToInventory(i);
    }
  }

  playerFile.close();
  return true;
}

void SaveManager::clearSaves() {
  remove(PLAYER_SAVE_FILE.c_str());
  for (int i = 1; i <= 9; ++i) {
    string filename = "SaveLevel" + to_string(i) + ".txt";
    remove(filename.c_str());
  }
}

bool SaveManager::saveExists() {
  return _access(PLAYER_SAVE_FILE.c_str(), 0) != -1;
}
