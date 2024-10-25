// ScreenList.h
#pragma once
#include <vector>
#include "IGameScreen.h"

namespace JAGEngine {
  class IMainGame;  // Change from MainGame

  class ScreenList {
  public:
    ScreenList(IMainGame* game);  // Change from MainGame
    ~ScreenList();

    IGameScreen* moveNext();
    IGameScreen* movePrevious();
    void setScreen(int nextScreen);
    void addScreen(IGameScreen* newScreen);
    void destroy();
    IGameScreen* getCurrent();

  protected:
    IMainGame* m_game = nullptr;  // Change from MainGame
    std::vector<IGameScreen*> m_screens;
    int m_currentScreenIndex = -1;
  };
}
