//Main.cpp

#include <iostream>
#include "MainGame.h"

int main(int argc, char* argv[]) {
  MainGame mainGame;
  mainGame.run();

  std::cout << "Enter any key to quit...\n";
  int a;
  std::cin >> a;
  return 0;
}
