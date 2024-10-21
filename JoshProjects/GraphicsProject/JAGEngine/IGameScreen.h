#pragma once

namespace JAGEngine {

  class MainGame;

  enum class ScreenState {
    NONE,
    RUNNING,
    EXIT_APPLICATION,
    CHANGE_NEXT,
    CHANGE_PREVIOUS
  };

  class IGameScreen {
  public:
    IGameScreen() {
      // Empty
    }
    virtual ~IGameScreen() {
      // Empty
    }

    virtual int getNextScreenIndex() const = 0;
    virtual int getPreviousScreenIndex() const = 0;

    // Called at beginning and end of app
    virtual void build() = 0;
    virtual void destroy() = 0;

    // Called when screen enters and exits focus
    virtual void onEntry() = 0;
    virtual void onExit() = 0;

    // Called in the main game loop
    virtual void update() = 0;
    virtual void draw() = 0;

    int getIndex() const {
      return m_screenIndex;
    }

    // Setters
    void setRunning() {
      m_currentState = ScreenState::RUNNING;
    }
    void setParentGame(MainGame* game) { m_game = game; }

    // Getters
    ScreenState getState() const { return m_currentState; }

  protected:
    ScreenState m_currentState = ScreenState::NONE;
    MainGame* m_game = nullptr;
    int m_screenIndex = -1; 
  };
}

