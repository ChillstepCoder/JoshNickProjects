// App.cpp
#include "App.h"
#include "JAGEngine/ScreenList.h"

void App::onInit() {
  // Any game-specific initialization
}

void App::addScreens() {
  if (!m_screenList) {
    std::cerr << "Screen list is null in addScreens!" << std::endl;
    return;
  }

  m_gameplayScreen = std::make_unique<GameplayScreen>();
  if (m_gameplayScreen) {
    m_screenList->addScreen(m_gameplayScreen.get());
    m_screenList->setScreen(m_gameplayScreen->getIndex());
  }
}

void App::onExit() {
  // Any game-specific cleanup
}
