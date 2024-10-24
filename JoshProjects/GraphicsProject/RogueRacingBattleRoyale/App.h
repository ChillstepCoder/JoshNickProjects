// App.h
#pragma once
#include <JAGEngine/IMainGame.h>
#include "GameplayScreen.h"

class App : public JAGEngine::IMainGame {
public:
  App() : IMainGame() {  // Explicitly call base constructor
    std::cout << "App constructor start\n";
  }
  ~App() {
    std::cout << "App destructor\n";
  }

  virtual void onInit() override;
  virtual void addScreens() override;
  virtual void onExit() override;

private:
  std::unique_ptr<GameplayScreen> m_gameplayScreen = nullptr;
};
