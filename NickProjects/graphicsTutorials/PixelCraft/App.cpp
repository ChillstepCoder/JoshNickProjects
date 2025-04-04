#include "App.h"
#include <Bengine/ScreenList.h>

App::App() {

}
App::~App() {

}

void App::onInit() {

}

void App::addScreens() {
    m_gameplayScreen = std::make_unique<GameplayScreen>(&m_window);
    m_mainMenuScreen = std::make_unique<MainMenuScreen>(&m_window);
    m_textureEditorScreen = std::make_unique<TextureEditorScreen>(&m_window);

    m_screenList->addScreen(m_mainMenuScreen.get());
    m_screenList->addScreen(m_gameplayScreen.get());
    m_screenList->addScreen(m_textureEditorScreen.get());
    m_screenList->setScreen(m_mainMenuScreen->getScreenIndex());
}

void App::onExit() {

}