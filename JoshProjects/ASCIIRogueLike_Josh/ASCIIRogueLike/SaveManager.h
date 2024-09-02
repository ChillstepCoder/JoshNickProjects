// SaveManager.h
#pragma once
#include <string>
#include "Player.h"

class SaveManager {
public:
  static void saveGame(const Player& player, int currentLevel);
  static bool loadGame(Player& player, int& currentLevel);
  static void clearSaves();
  static bool saveExists();

private:
  static const std::string SAVE_FILE_PREFIX;
  static const std::string PLAYER_SAVE_FILE;
};
