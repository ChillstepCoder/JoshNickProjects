// IGameScreen.h
#pragma once

namespace JAGEngine {
  class IMainGame;  // Change from MainGame
  class ScreenList;

  enum class ScreenState {
    NONE,
    RUNNING,
    EXIT_APPLICATION,
    CHANGE_NEXT,
    CHANGE_PREVIOUS
  };

  class IGameScreen {
    friend class ScreenList;
  public:
    IGameScreen() {
      // Empty
    }
    virtual ~IGameScreen() {
      // Empty
    }

    virtual int getNextScreenIndex() const = 0;
    virtual int getPreviousScreenIndex() const = 0;
    virtual void build() = 0;
    virtual void destroy() = 0;
    virtual void onEntry() = 0;
    virtual void onExit() = 0;
    virtual void update() = 0;
    virtual void draw() = 0;

    int getIndex() const {
      return m_screenIndex;
    }

    void setRunning() {
      m_currentState = ScreenState::RUNNING;
    }

    void setParentGame(IMainGame* game) {  // Change from MainGame to IMainGame
      m_game = game;
    }

    ScreenState getState() const { return m_currentState; }

  protected:
    ScreenState m_currentState = ScreenState::NONE;
    IMainGame* m_game = nullptr;  // Change from MainGame to IMainGame
    int m_screenIndex = -1;
  };
}
