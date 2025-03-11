// Main.cpp - Racing Game

#include <JAGEngine/IMainGame.h>

#include "App.h"
#include <cstdlib>
#include <ctime>  

int main(int argc, char** argv) {
  std::srand(static_cast<unsigned int>(std::time(nullptr)));
  App app;
  app.run();
  return 0;
}

