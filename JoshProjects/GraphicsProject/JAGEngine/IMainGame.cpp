// IMainGame.cpp

#include "IMainGame.h"
#include "Timing.h"
#include <iostream>
#include "ScreenList.h"
#include "IGameScreen.h"

namespace JAGEngine {

  IMainGame::IMainGame() {
    std::cout << "IMainGame constructor start\n";
    try {
      m_screenList = std::make_unique<ScreenList>(this);
      std::cout << "Created screen list\n";
    }
    catch (const std::exception& e) {
      std::cerr << "Exception creating screen list: " << e.what() << std::endl;
      throw;
    }
    m_isRunning = false;
    m_fps = 0.0f;
    std::cout << "IMainGame constructor complete\n";
  }

  IMainGame::~IMainGame() {
    signalCleanup();
    // Ensure proper cleanup
    if (m_screenList) {
      m_screenList->destroy();
      m_screenList.reset();
    }
  }

  void IMainGame::cleanup() {
    if (m_imguiInitialized) {
      cleanupImGui();
    }

    if (m_screenList) {
      m_currentScreen = nullptr;
      m_screenList->destroy();
      m_screenList.reset();
    }

    m_window.destroy();
    SDL_Quit();
  }

  void IMainGame::run() {
    if (!init()) return;

    std::cout << "Starting game loop...\n";

    FpsLimiter limiter;
    limiter.setMaxFPS(60.0f);
    m_isRunning = true;

    // Ensure window is visible before entering game loop
    SDL_RaiseWindow(m_window.getSDLWindow());

    while (m_isRunning) {
      // Process all SDL events
      SDL_Event evnt;
      while (SDL_PollEvent(&evnt)) {
        if (m_imguiInitialized) {
          ImGui_ImplSDL2_ProcessEvent(&evnt);
        }
        if (evnt.type == SDL_WINDOWEVENT) {
          switch (evnt.window.event) {
          case SDL_WINDOWEVENT_MINIMIZED:
            std::cout << "Window minimized\n";
            break;
          case SDL_WINDOWEVENT_RESTORED:
            std::cout << "Window restored\n";
            SDL_RaiseWindow(m_window.getSDLWindow());
            break;
          }
        }
        onSDLEvent(evnt);
      }

      beginImGuiFrame();

      m_inputManager.update();
      update();
      draw();

      endImGuiFrame();

      m_fps = limiter.end();
      m_window.swapBuffer();
    }
  }

  void IMainGame::exitGame() {
    std::cout << "exitGame called!\n";

    // Let the current screen cleanup first
    if (m_currentScreen) {
      m_currentScreen->onExit();
    }

    // Then cleanup screen list
    if (m_screenList) {
      m_screenList->destroy();
      m_screenList.reset();
    }

    // Finally, stop the game loop
    signalCleanup();
    m_isRunning = false;

  }

  void IMainGame::onSDLEvent(SDL_Event& evnt) {
    switch (evnt.type) {
    case SDL_QUIT:
      std::cout << "SDL_QUIT received\n";
      m_isRunning = false;
      break;
    case SDL_KEYDOWN:
      std::cout << "SDL_KEYDOWN received: " << evnt.key.keysym.sym << "\n";
      m_inputManager.pressKey(evnt.key.keysym.sym);
      break;
    case SDL_KEYUP:
      std::cout << "SDL_KEYUP received: " << evnt.key.keysym.sym << "\n";
      m_inputManager.releaseKey(evnt.key.keysym.sym);
      break;
    case SDL_MOUSEMOTION:
      m_inputManager.setMouseCoords((float)evnt.motion.x, (float)evnt.motion.y);
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

    if (!initSystems()) return false;
    if (!initImGui()) return false;

    onInit();
    addScreens();

    m_currentScreen = m_screenList->getCurrent();
    if (m_currentScreen) {
      m_currentScreen->onEntry();
      m_currentScreen->setRunning();
    }

    return true;
  }

  bool IMainGame::initSystems() {
    // Set proper SDL window flags
    const Uint32 flags = SDL_WINDOW_OPENGL | SDL_WINDOW_FULLSCREEN/* | SDL_WINDOW_BORDERLESS*/;

    m_window.create("Racing Game", 1920, 1080, flags);

    // Initialize OpenGL settings
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    return true;
  }

  bool IMainGame::initImGui() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    if (!ImGui_ImplSDL2_InitForOpenGL(m_window.getSDLWindow(), m_window.getGLContext()) ||
      !ImGui_ImplOpenGL3_Init("#version 130")) {
      return false;
    }

    m_imguiInitialized = true;
    return true;
  }

  void IMainGame::cleanupImGui() {
    if (m_imguiInitialized) {
      ImGui_ImplOpenGL3_Shutdown();
      ImGui_ImplSDL2_Shutdown();
      ImGui::DestroyContext();
      m_imguiInitialized = false;
    }
  }

  void IMainGame::beginImGuiFrame() {
    if (m_imguiInitialized) {
      ImGui_ImplOpenGL3_NewFrame();
      ImGui_ImplSDL2_NewFrame();
      ImGui::NewFrame();
    }
  }

  void IMainGame::endImGuiFrame() {
    if (m_imguiInitialized) {
      ImGui::Render();
      ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }
  }

  void IMainGame::update() {
    std::cout << "IMainGame::update() called\n";
    if (m_currentScreen) {
      m_inputManager.update();  // Make sure this is being called!
      std::cout << "Input manager updated\n";

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

