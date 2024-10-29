// App.cpp
#include "App.h"
#include "JAGEngine/ScreenList.h"

void App::onInit() {
  // Any game-specific initialization
}

void App::addScreens() {
  m_gameplayScreen = std::make_unique<GameplayScreen>();
  m_levelEditorScreen = std::make_unique<LevelEditorScreen>();

  m_screenList->addScreen(m_gameplayScreen.get());
  m_screenList->addScreen(m_levelEditorScreen.get());

  m_screenList->setScreen(0);  // Start with gameplay screen
}

void App::onExit() {
  // Any game-specific cleanup
}
