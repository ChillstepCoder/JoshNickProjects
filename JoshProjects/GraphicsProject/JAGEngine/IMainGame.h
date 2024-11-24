// IMainGame.h
#pragma once
#include <memory>
#include "Window.h"
#include "JAGEngine.h"
#include "InputManager.h"
#include <ImGui/imgui.h>
#include <ImGui/imgui_impl_sdl2.h>
#include <ImGui/imgui_impl_opengl3.h>

namespace JAGEngine {
  class ScreenList;
  class IGameScreen;

  class IMainGame {
  public:
    IMainGame();
    virtual ~IMainGame();
    void run();
    void exitGame();
    InputManager& getInputManager() { return m_inputManager; }
    Window& getWindow() { return m_window; }
    const Window& getWindow() const { return m_window; }

    virtual void onInit() = 0;
    virtual void addScreens() = 0;
    virtual void onExit() = 0;

    const float getFps() const { return m_fps; }

    void signalCleanup() {
      if (!m_isCleanedUp) {
        cleanup();
        m_isCleanedUp = true;
      }
    }


  protected:
    virtual void update();
    virtual void draw();
    void onSDLEvent(SDL_Event& evnt);
    bool init();
    bool initSystems();
    bool initImGui();
    void cleanupImGui();
    void beginImGuiFrame();
    void endImGuiFrame();
    bool m_isCleanedUp = false;
    void cleanup();

    std::unique_ptr<ScreenList> m_screenList;
    IGameScreen* m_currentScreen = nullptr;
    bool m_isRunning = false;
    float m_fps = 0.0f;
    Window m_window;
    InputManager m_inputManager;
    bool m_imguiInitialized = false;
  };
}
