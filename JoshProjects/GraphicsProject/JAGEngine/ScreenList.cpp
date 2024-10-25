//ScreenList.cpp

#include "ScreenList.h"
#include "IGameScreen.h"
#include "IMainGame.h"
#include <iostream>

namespace JAGEngine {

  ScreenList::ScreenList(IMainGame* game) :  // Change from MainGame
    m_game(game),
    m_currentScreenIndex(-1)
  {
    if (!game) {
      throw std::runtime_error("Null game pointer in ScreenList constructor");
    }
  }

  ScreenList::~ScreenList() {
    destroy();
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
    if (nextScreen >= 0 && nextScreen < m_screens.size()) {
      m_currentScreenIndex = nextScreen;
    }
  }
  void ScreenList::addScreen(IGameScreen* newScreen) {
    if (newScreen) {
      newScreen->m_screenIndex = m_screens.size();
      m_screens.push_back(newScreen);
      newScreen->build();
      newScreen->setParentGame(m_game);
    }
  }

  void ScreenList::destroy() {
    for (size_t i = 0; i < m_screens.size(); i++) {
      m_screens[i]->destroy();
    }
    m_screens.clear();
    m_currentScreenIndex = -1;
  }

  IGameScreen* ScreenList::getCurrent() {
    if (m_screens.empty()) {
      return nullptr;
    }
    return m_screens[m_currentScreenIndex];
  }
}
