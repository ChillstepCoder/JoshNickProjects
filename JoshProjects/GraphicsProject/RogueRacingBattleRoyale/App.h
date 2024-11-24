// App.h
#pragma once
#include <JAGEngine/IMainGame.h>
#include "GameplayScreen.h"
#include "LevelEditorScreen.h"

class App : public JAGEngine::IMainGame {
public:
  App() : IMainGame() {
    std::cout << "App constructor start\n";
  }
  ~App() {
    std::cout << "App destructor\n";
    signalCleanup();
  }
  virtual void onInit() override;
  virtual void addScreens() override;
  virtual void onExit() override;
private:
  std::unique_ptr<GameplayScreen> m_gameplayScreen = nullptr;
  std::unique_ptr<LevelEditorScreen> m_levelEditorScreen = nullptr;
};
