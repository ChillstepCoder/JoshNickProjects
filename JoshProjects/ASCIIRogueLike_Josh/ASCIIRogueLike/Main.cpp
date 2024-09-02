//Main.cpp

#include <iostream>
#include <string>
#include <fstream>
#include <ctime>

#include "GameSystem.h"
#include "Player.h"

using namespace std;

int main() {
  srand(static_cast<unsigned int>(time(0)));
  GameSystem gameSystem;
  gameSystem.playGame();
  
  cin.ignore();
  cin.get();
  return 0;
}

// W,A,S,D - Movement
// 'P' - Potion
// 'F' - Food
