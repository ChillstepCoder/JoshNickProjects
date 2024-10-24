// Main.cpp

#include <JAGEngine/IMainGame.h>

#include "App.h"

int main(int argc, char** argv) {
  try {
    App app;
    app.run();
  }
  catch (const std::exception& e) {
    std::cerr << "Exception in main: " << e.what() << std::endl;
    return 1;
  }
  catch (...) {
    std::cerr << "Unknown exception in main" << std::endl;
    return 1;
  }
  return 0;
}
