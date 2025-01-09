// App.cpp
#include "App.h"
#include "JAGEngine/ScreenList.h"

void App::onInit() {
  m_audioEngine = std::make_unique<AudioEngine>();
  if (!m_audioEngine->init()) {
    std::cout << "Failed to initialize racing audio engine!\n";
  }
}

void App::addScreens() {
  m_gameplayScreen = std::make_unique<GameplayScreen>();
  m_levelEditorScreen = std::make_unique<LevelEditorScreen>();

  m_screenList->addScreen(m_gameplayScreen.get());
  m_screenList->addScreen(m_levelEditorScreen.get());

  m_screenList->setScreen(0);  // Start with gameplay screen
}

void App::onExit() {
  if (m_audioEngine) {
    m_audioEngine->cleanup();
  }
}
