#include "ScreenList.h"
#include "IGameScreen.h"

namespace JAGEngine {

  ScreenList::ScreenList(MainGame* game) :
  m_game(game) {
    //empty
  }

  IGameScreen* ScreenList::moveNext() {
    IGameScreen* currentScreen = getCurrent();
    if (currentScreen->getNextScreenIndex() != -1) {
      m_currentScreenIndex = currentScreen->getNextScreenIndex();
    }
    return getCurrent();
  }
  IGameScreen* ScreenList::movePrevious() {
    IGameScreen* currentScreen = getCurrent();
    if (currentScreen->getPreviousScreenIndex() != -1) {
      m_currentScreenIndex = currentScreen->getPreviousScreenIndex();
    }
    return getCurrent();
  }

  void ScreenList::setScreen(int nextScreen) {
    m_currentScreenIndex = nextScreen;
  }
  void ScreenList::addScreen(IGameScreen* newScreen) {
    newScreen->m_screenIndex = m_screens.size();
    m_screens.push_back(newScreen);
    newScreen->build();
    newScreen->setParentGame(m_game);
  }

  void ScreenList::destroy() {
    for (size_t i = 0; i < m_screens.size(); i++) {
      m_screens[i]->destroy();
    }
    m_screens.resize(0);
    m_currentScreenIndex = -1;
  }

  ScreenList::~ScreenList() {
    destroy();
  }

  IGameScreen* ScreenList::getCurrent() {
    if (m_currentScreenIndex = -1) return nullptr;
    return m_screens[m_currentScreenIndex];
  }
}
