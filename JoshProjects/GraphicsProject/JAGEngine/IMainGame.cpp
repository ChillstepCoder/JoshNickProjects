// IMainGame.cpp

#include "IMainGame.h"
#include "Timing.h"
#include <iostream>
#include "ScreenList.h"
#include "IGameScreen.h"

namespace JAGEngine {

  IMainGame::IMainGame() {
    std::cout << "IMainGame constructor start\n";
    m_screenList = std::make_unique<ScreenList>(this);
    /*try {
      m_screenList = std::make_unique<ScreenList>(this);
      std::cout << "Created screen list\n";
    }
    catch (const std::exception& e) {
      std::cerr << "Exception creating screen list: " << e.what() << std::endl;
      throw;
    }*/
    m_isRunning = false;
    m_fps = 0.0f;
    std::cout << "IMainGame constructor complete\n";
  }

  IMainGame::~IMainGame() {
    // Ensure proper cleanup
    if (m_screenList) {
      m_screenList->destroy();
      m_screenList.reset();
    }
  }

  void IMainGame::run() {
    std::cout << "Run start\n";
    if (!init()) {
      std::cerr << "Failed to initialize!\n";
      return;
    }

    FpsLimiter limiter;
    limiter.setMaxFPS(60.0f);

    m_isRunning = true;
    while (m_isRunning) {
      limiter.begin();

      m_inputManager.update();
      update();
      draw();

      m_fps = limiter.end();
      m_window.swapBuffer();
    }

  }

  void IMainGame::exitGame() {
    m_currentScreen->onExit();
    if (m_screenList) {
      m_screenList->destroy();
      m_screenList.reset();
    }
    m_isRunning = false;
  }

  void IMainGame::onSDLEvent(SDL_Event& evnt) {
    switch (evnt.type) {
    case SDL_QUIT:
      m_isRunning = false;
      break;
    case SDL_MOUSEMOTION:
        m_inputManager.setMouseCoords((float)evnt.motion.x, (float)evnt.motion.y);
      break;
    case SDL_KEYDOWN:
      m_inputManager.pressKey(evnt.key.keysym.sym);
      break;
    case SDL_KEYUP:
      m_inputManager.releaseKey(evnt.key.keysym.sym);
      break;
    case SDL_MOUSEBUTTONDOWN:
        m_inputManager.pressKey(evnt.button.button);
      break;
    case SDL_MOUSEBUTTONUP:
        m_inputManager.releaseKey(evnt.button.button);
      break;
    }
  }

  bool IMainGame::init() {
    std::cout << "Init start\n";
    JAGEngine::init();
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);

    if (!initSystems()) {
      std::cerr << "Failed to initialize systems\n";
      return false;
    }

    std::cout << "Systems initialized, calling onInit()\n";
    onInit();

    std::cout << "Checking screen list before addScreens()\n";
    if (!m_screenList) {
      std::cerr << "Screen list is null before addScreens!\n";
      return false;
    }

    std::cout << "Calling addScreens()\n";
    addScreens();

    std::cout << "Getting current screen\n";
    m_currentScreen = m_screenList->getCurrent();
    if (!m_currentScreen) {
      std::cerr << "Current screen is null after addScreens!\n";
      return false;
    }

    std::cout << "Calling onEntry()\n";
    m_currentScreen->onEntry();
    m_currentScreen->setRunning();

    std::cout << "Init complete\n";
    return true;
  }

  bool IMainGame::initSystems() {
    m_window.create("Default", 1920, 1080, 0);
    return true;
  }

  void IMainGame::update() {
    if (m_currentScreen) {
      switch (m_currentScreen->getState()) {
        case ScreenState::RUNNING:
          m_currentScreen->update();
          break;
        case ScreenState::CHANGE_NEXT:
          m_currentScreen->onExit();
          m_currentScreen = m_screenList->moveNext();
          if (m_currentScreen) {
            m_currentScreen->setRunning();
            m_currentScreen->onEntry();
          }
          break;
        case ScreenState::CHANGE_PREVIOUS:
          m_currentScreen->onExit();
          m_currentScreen = m_screenList->movePrevious();
          if (m_currentScreen) {
            m_currentScreen->setRunning();
            m_currentScreen->onEntry();
          }
          break;
        case ScreenState::EXIT_APPLICATION:
          exitGame();
          break;
        default:
          break;
      }
    } else {
      exitGame();
    }
  }

  void IMainGame::draw() {
    glViewport(0, 0, m_window.getScreenWidth(), m_window.getScreenHeight());
    if (m_currentScreen && m_currentScreen->getState() == ScreenState::RUNNING) {
      m_currentScreen->draw();
    }
  }

}
