//Level.cpp
#include "GameSystem.h"
#include "Level.h"
#include "Player.h"
#include "Enemy.h"
#include <fstream>
#include <iostream> 
#include <cstdio>
#include <string>
#include <vector>
#include "Shop.h"
#include <random>
#include <chrono>


Level::Level() : _currentLevel(0), _player(nullptr), gameSystem(nullptr) {
}

bool Level::load(int num, Player& player) {
  _levelData.clear();
  _enemies.clear();
  ifstream inputFile;
  string saveFilename = "SaveLevel" + to_string(num) + ".txt";
  string originalFilename = "Level" + to_string(num) + ".txt";

  // Try to open the save file first
  inputFile.open(saveFilename);
  if (!inputFile) {
    inputFile.open(originalFilename);
    if (!inputFile) {
      perror(originalFilename.c_str());
      return false;
    }
  }

  string input;
  while (getline(inputFile, input)) {
    _levelData.push_back(input + "\n");
  }

  inputFile.close();

  bool setPos = false;
  char tile;
  for (int i = 0; i < _levelData.size(); i++) {
    for (int ii = 0; ii < _levelData[i].size(); ii++) {
      tile = _levelData[i][ii];

      switch (tile) {
      case '@': //Player
        setPos = true;
        player.setPosition(ii, i);
        break;
      case 's': //snake
        _enemies.push_back(Enemy("Snake", tile, 1, 1, 1, 4, 2, 2, 1, true));
        // name, tile, level, attack, defense, health, xp, range, gold mobile?
        _enemies.back().setPosition(ii, i);
        break;
      case 'g': //Goblin
        _enemies.push_back(Enemy("Goblin", tile, 4, 3, 3, 8, 5, 4, 3, true));
        // name, tile, level, attack, defense, health, xp, range, gold mobile?
        _enemies.back().setPosition(ii, i);
        break;
      case 'G': // Guard
        _enemies.push_back(Enemy("Guard", tile, 8, 4, 5, 12, 10, 0, 5, false));
        // name, tile, level, attack, defense, health, xp, range, gold mobile?
        _enemies.back().setPosition(ii, i);
        break;
      case 'O': //Ogre
        _enemies.push_back(Enemy("Ogre", tile, 10, 10, 10, 18, 15, 5, 10, true));
        // name, tile, level, attack, defense, health, xp, range, gold, mobile?
        _enemies.back().setPosition(ii, i);
        break;
      case 'K': // Knight
        _enemies.push_back(Enemy("Knight", tile, 15, 15, 15, 25, 25, 0, 15, false));
        // name, tile, level, attack, defense, health, xp, range, gold mobile?
        _enemies.back().setPosition(ii, i);
        break;
      case 'W': // Warrior
        _enemies.push_back(Enemy("Warrior", tile, 20, 25, 15, 30, 25, 7, 20, true));
        // name, tile, level, attack, defense, health, xp, range, gold mobile?
        _enemies.back().setPosition(ii, i);
        break;
      case 'P': // Paladin
        _enemies.push_back(Enemy("Paladin", tile, 20, 20, 20, 40, 30, 0, 25, false));
        // name, tile, level, attack, defense, health, xp, range, gold mobile?
        _enemies.back().setPosition(ii, i);
        break;
      case 'T': // Troll
        _enemies.push_back(Enemy("Troll", tile, 25, 25, 25, 50, 45, 6, 30, true));
        // name, tile, level, attack, defense, health, xp, range, gold mobile?
        _enemies.back().setPosition(ii, i);
        break;
      case 'D': //Dragon
        _enemies.push_back(Enemy("Dragon", tile, 30, 30, 30, 100, 70, 10, 50, true));
        // name, tile, level, attack, defense, health, xp, range, gold mobile?
        _enemies.back().setPosition(ii, i);
        break;
      }
    }
  }
    _currentLevel = num;

    // Find player position
    int playerX = -1, playerY = -1;
    for (int i = 0; i < _levelData.size(); i++) {
      for (int j = 0; j < _levelData[i].size(); j++) {
        if (_levelData[i][j] == '@') {
          playerX = j;
          playerY = i;
          _levelData[i][j] = '.';
          break;
        }
      }
      if (playerX != -1) break;
    }

    // If player position not found in level data, use player's current position
    if (playerX == -1 || playerY == -1) {
      player.getPosition(playerX, playerY);
    }

    player.setPosition(playerX, playerY);
    return true;
}

bool Level::save(int num) {
  ofstream outputFile;
  string filename = "SaveLevel" + to_string(num) + ".txt";
  outputFile.open(filename);
  if (!outputFile) {
    perror(filename.c_str());
    return false;
  }

  // Create a copy of _levelData
  vector<string> levelDataCopy = _levelData;
  for (auto& row : levelDataCopy) {
    for (char& tile : row) {
      if (tile == '@') {
        tile = '.';
      }
    }
  }

  for (const auto& row : levelDataCopy) {
    outputFile << row;
  }
  outputFile.close();
  return true;
}

char Level::getTile(int x, int y) const {
  if (y >= 0 && y < _levelData.size() && x >= 0 && x < _levelData[y].size()) {
    return _levelData[y][x];
  }
  return '#';
}

string Level::generateLevel(int seed) {
  _levelData.clear();

  for (int i = 0; i < levelHeight; i++) {
    string row;
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

  string levelString;
  for (const auto& row : _levelData) {
    levelString += row;
  }
  return levelString;
}

void Level::printVoid() {
  system("cls");
}

void Level::printStats() {
  int level = _player->getLevel();
  int attack = _player->getAttack();
  int defense = _player->getDefense();
  int luck = _player->getLuck();
  int attackBonus = _player->getAttackBonus();
  int defenseBonus = _player->getDefenseBonus();
  int luckBonus = _player->getLuckBonus();
  int xp = _player->getExperience();
  int xpto = _player->getXpToNextLevel();
  int gold = _player->getGold();
  int keys = _player->getKeys();

  std::string attackStr = std::to_string(attack);
  std::string defenseStr = std::to_string(defense);
  std::string luckStr = std::to_string(luck);

  if (attackBonus != 0) attackStr += "(" + std::string(attackBonus > 0 ? "+" : "") + std::to_string(attackBonus) + ")";
  if (defenseBonus != 0) defenseStr += "(" + std::string(defenseBonus > 0 ? "+" : "") + std::to_string(defenseBonus) + ")";
  if (luckBonus != 0) luckStr += "(" + std::string(luckBonus > 0 ? "+" : "") + std::to_string(luckBonus) + ")";

  printf("LEVEL: %d  ATK: %s  DEF: %s  LUCK: %s  GOLD: %d  KEYS: %d\n",
    level, attackStr.c_str(), defenseStr.c_str(), luckStr.c_str(), gold, keys);

  printf("FOOD: %d  POTIONS: %d\n", _player->getItemCount(4), _player->getItemCount(5));

  // XP bar
  int xpPerc = (xpto > 0) ? (int)((float)xp / (float)xpto * 80) : 0;
  std::string xpbar = std::string(xpPerc, '=') + std::string(80 - xpPerc, '-');
  printf("XP: %d / %d\n", xp, xpto);
  printf("%s\n", xpbar.c_str());
}

void Level::printStats2() {
  int health = _player->getHealth();
  int healthMax = _player->getMaxHealth();
  int hpPerc = (healthMax > 0) ? (int)((float)health / (float)healthMax * 80) : 0;
  std::string hpbar = std::string(hpPerc, '=') + std::string(80 - hpPerc, '-');

  printf("%s\n", hpbar.c_str());
  printf("HP: %d / %d\n", health, healthMax);
}

void Level::print() {
  if (!_player) return;

  int playerX, playerY;
  _player->getPosition(playerX, playerY);

  std::string levelString;
  levelString.reserve(_levelData.size() * (_levelData[0].size() + 1));

  for (int y = 0; y < _levelData.size(); y++) {
    for (int x = 0; x < _levelData[y].size(); x++) {
      if (x == playerX && y == playerY) {
        levelString += '@';
      }
      else {
        levelString += _levelData[y][x];
      }
    }
  }

  printf("%s", levelString.c_str());
}
void Level::setTile(int x, int y, char tile) {
  if (y >= 0 && y < _levelData.size() && x >= 0 && x < _levelData[y].size()) {
    _levelData[y][x] = tile;
  }
}

void Level::placePlayer(const Player& player) {
  int x, y;
  player.getPosition(x, y);
  if (x >= 0 && x < this->levelWidth && y >= 0 && y < this->levelHeight) {
    this->_levelData[y][x] = '@';
  }
}

void Level::setGameSystem(GameSystem* gs) {
  gameSystem = gs;
}

void Level::clearPlayer() {
  for (auto& row : _levelData) {
    for (char& tile : row) {
      if (tile == '@') {
        tile = '.';
      }
    }
  }
}

void Level::processPlayerMove(Player& player, int targetX, int targetY) {
  int playerX, playerY;
  player.getPosition(playerX, playerY);
  char moveTile = getTile(targetX, targetY);

  switch (moveTile) {
  case '#':
    break;
  case '.':
  case '*':
  case '+':
  case '_':
  case '|':
    if (moveTile == '*') {
      interactWithFood(player, targetX, targetY);
    }
    else if (moveTile == '+') {
      interactWithKey(player, targetX, targetY);
    }
    else if (moveTile == '_' || moveTile == '|') {
      interactWithDoor(player, targetX, targetY);
      if (getTile(targetX, targetY) != '.') {
        break;
      }
    }
    player.setPosition(targetX, targetY);
    setTile(playerX, playerY, '.');
    setTile(targetX, targetY, '@');
    break;
  case 'g':
  case 's':
  case 'O':
  case 'D':
  case 'G':
  case 'K':
  case 'P':
  case 'W':
  case 'T':
    battleMonster(player, targetX, targetY);
    break;
  case 'S':
    interactWithShop(player);
    break;
  default:
    std::cout << "Unknown tile type" << std::endl;
    break;
  }
}

void Level::movePlayer(char input, Player& player) {
  int playerX, playerY;
  player.getPosition(playerX, playerY);

  int newX = playerX;
  int newY = playerY;

  switch (input) {
  case 'w': case 'W': newY--; break;
  case 's': case 'S': newY++; break;
  case 'a': case 'A': newX--; break;
  case 'd': case 'D': newX++; break;
  default: return;
  }
  if (newX >= 0 && newX < levelWidth && newY >= 0 && newY < levelHeight) {
    if (_levelData[newY][newX] == '.') {
      _levelData[playerY][playerX] = '.';
      _levelData[newY][newX] = '@';
      player.setPosition(newX, newY);
    }
    else if (_levelData[newY][newX] != '#') {
      processPlayerMove(player, newX, newY);
    }
  }
}

void Level::updateEnemies(Player& player) {
  if (!_enemiesCanMove) return;
  int playerX, playerY;
  int enemyX, enemyY;

  char aiMove;

  player.getPosition(playerX, playerY);

  for (int i = 0; i < _enemies.size(); i++) {
    if (_enemies[i].isMobile()) {
      aiMove = _enemies[i].getMove(playerX, playerY);
      _enemies[i].getPosition(enemyX, enemyY);

      int newX = enemyX;
      int newY = enemyY;

      switch (aiMove) {
      case 'w': newY--; break;
      case 's': newY++; break;
      case 'a': newX--; break;
      case 'd': newX++; break;
      }
      if (newX >= 0 && newX < levelWidth && newY >= 0 && newY < levelHeight) {
        if (_levelData[newY][newX] != '#') {
          processEnemyMove(player, i, newX, newY);
        }
      }
    }
  }
}

void Level::processEnemyMove(Player& player, int enemyIndex, int targetX, int targetY) {
  int playerX, playerY;
  int enemyX, enemyY;

  _enemies[enemyIndex].getPosition(enemyX, enemyY);
  player.getPosition(playerX, playerY);

  char moveTile = getTile(targetX, targetY);

  switch (moveTile) {
  case '#':
    break;
  case '@':
    battleMonster(player, enemyX, enemyY);
    break;
  case '.':
    _enemies[enemyIndex].setPosition(targetX, targetY);
    setTile(enemyX, enemyY, '.');
    setTile(targetX, targetY, _enemies[enemyIndex].getTile());
    break;
  default:
    break;
  }
}

void Level::battleMonster(Player& player, int targetX, int targetY) {
  int enemyX, enemyY;
  int attackRoll;
  int attackResult;
  int playerX, playerY;

  string enemyName;
  player.getPosition(playerX, playerY);

  bool enemyFound = false;
  for (int i = 0; i < _enemies.size(); i++) {
    _enemies[i].getPosition(enemyX, enemyY);
    enemyName = _enemies[i].getName();

    if (targetX == enemyX && targetY == enemyY) {
      enemyFound = true;
      // Player attacks
      attackRoll = player.attack();
      printVoid();
      printStats();
      print();
      printStats2();
      printf("Player attacked %s with a roll of %d. ", enemyName.c_str(), attackRoll);
      if (attackRoll > 0) {
        int enemyDefense = _enemies[i].defenseRoll();
        int damageToEnemy = (int)((float)attackRoll / (float)enemyDefense);
        attackResult = _enemies[i].takeDamage(damageToEnemy);
        if (attackResult != 0) {
          int xpGained = _enemies[i].getXP();
          int goldGained = _enemies[i].getGold();
          int luckBonus = std::min(player.getLuck(), goldGained);
          static default_random_engine randomEngine(time(NULL));
          std::uniform_int_distribution<int> goldRoll(goldGained, goldGained + luckBonus);
          goldGained = goldRoll(randomEngine);

          player.addGold(goldGained);

          player.getPosition(playerX, playerY);
          setTile(playerX, playerY, '.');
          player.setPosition(targetX, targetY);
          setTile(targetX, targetY, '@');

          printVoid();
          printStats();
          print();
          printStats2();
          printf("%s took %d damage and died! You gained %d experience and %d gold.\n",
            enemyName.c_str(), damageToEnemy, xpGained, goldGained);
          _enemies[i] = _enemies.back();
          _enemies.pop_back();
          i--;
          player.addExperience(xpGained);
          player.addGold(goldGained);
          system("PAUSE");
          return;
        }
        else if (damageToEnemy > 0) {
          printf("%s took %d damage\n", enemyName.c_str(), damageToEnemy);
        }
        else {
          printf("%s took no damage\n", enemyName.c_str());
        }
      }
      else {
        printf(" and missed!\n");
      }

      // Enemy attacks
      attackRoll = _enemies[i].attack();
      printf("%s attacked player with a roll of %d. ", enemyName.c_str(), attackRoll);
      if (attackRoll > 0) {
        int playerDefense = player.defenseRoll();
        int damageToPlayer = attackRoll / playerDefense;
        if (damageToPlayer > 0) {
          player.takeDamage(damageToPlayer);
          printf("Player took %d damage.\n", damageToPlayer);

          if (player.getHealth() <= 0) {
            setTile(playerX, playerY, 'x');
            printf("You died!\n");
            system("PAUSE");
            if (gameSystem) {
              gameSystem->gameOver();
            }
            return;
          }
        }
        else {
          printf("Player took no damage.\n");
        }
      }
      else {
        printf(" and missed!\n");
      }

      system("PAUSE");
      printVoid();
      printStats();
      print();
      printStats2();
      return;
    }
  }
}

void Level::interactWithShop(Player& player) {
  Shop shop;
  int choice;
  bool shopping = true;

  bool couldEnemiesMove = _enemiesCanMove;
  _enemiesCanMove = false;

  while (shopping) {
    printVoid();
    printStats();
    shop.printShop();
    shop.printInventory(player.getInventory());
    //std::cout << "\nYour gold: " << player.getGold() << "\n";
    std::cout << "Enter the number of the item you want to buy (0 to exit): ";

    if (!(std::cin >> choice)) {
      std::cin.clear();
      std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
      std::cout << "Invalid input. Please enter a number." << std::endl;
      system("PAUSE");
      continue;
    }

    if (choice == 0) {
      shopping = false;
    }
    else if (choice >= 1 && choice <= Shop::numItems) {
      int cost = Shop::itemPrices[choice - 1];
      if (player.canAddItem(choice - 1) && player.spendGold(cost)) {
        player.addToInventory(choice - 1);
        std::cout << "You bought " << shop.getItemName(choice - 1) << " for " << cost << " gold!\n";
      }
      else if (!player.canAddItem(choice - 1)) {
        std::cout << "You can't carry any more of this item!\n";
      }
      else {
        std::cout << "Not enough gold!\n";
      }
    }
    else {
      std::cout << "Invalid choice. Please enter a number between 0 and " << Shop::numItems << ".\n";
    }

    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    system("PAUSE");
  }

  _enemiesCanMove = couldEnemiesMove;
  printVoid();
  printStats();
  print();
  printStats2();
}

void Level::interactWithFood(Player& player, int x, int y) {
  player.heal(5);
  std::cout << "You ate some food and recovered 5 health!" << std::endl;
}

void Level::interactWithKey(Player& player, int x, int y) {
  player.addKey();
  std::cout << "You picked up a key!" << std::endl;
}

void Level::interactWithDoor(Player& player, int x, int y) {
  if (player.useKey()) {
    setTile(x, y, '.');
    std::cout << "You unlocked the door!" << std::endl;
  }
  else {
    std::cout << "You need a key to unlock this door." << std::endl;
  }
}
