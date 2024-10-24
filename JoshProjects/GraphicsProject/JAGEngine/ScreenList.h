//ScreenList.h

#pragma once
#include <iostream>
#include <vector>

namespace JAGEngine {
  class MainGame;
  class IGameScreen;
  class ScreenList
  {
  public:
    ScreenList(MainGame* game);
    ~ScreenList();

    IGameScreen* moveNext();
    IGameScreen* movePrevious();

    void setScreen(int nextScreen);
    void addScreen(IGameScreen* newScreen);

    void destroy();

    IGameScreen* getCurrent();

  protected:
    MainGame* m_game = nullptr;
    std::vector<IGameScreen*> m_screens;
    int m_currentScreenIndex = -1;
  };
}

