//App.h

#include <JAGEngine/IMainGame.h>
#include "GameplayScreen.h"

#pragma once
class App : public JAGEngine::IMainGame {
public:
  App();
  ~App();

  virtual void onInit() override;

  virtual void addScreens() override;

  virtual void onExit() override;
private:
  std::unique_ptr<GameplayScreen> m_gameplayScreen = nullptr;
};

