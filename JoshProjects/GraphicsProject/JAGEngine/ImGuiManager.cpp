//ImGuiManager.cpp

#include "ImGuiManager.h"
#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_sdl2.h"
#include "ImGui/imgui_impl_opengl3.h"
#include <GL/glew.h>

namespace JAGEngine {

  bool ImGuiManager::s_initialized = false;

  void ImGuiManager::init(Window* window) {
    if (s_initialized) return;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    ImGui::StyleColorsDark();

    ImGui_ImplSDL2_InitForOpenGL(window->getSDLWindow(), SDL_GL_GetCurrentContext());
    ImGui_ImplOpenGL3_Init("#version 130");

    s_initialized = true;
  }

  void ImGuiManager::shutdown() {
    if (!s_initialized) return;

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    s_initialized = false;
  }

  void ImGuiManager::newFrame() {
    if (!s_initialized) return;

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();
  }

  void ImGuiManager::render() {
    if (!s_initialized) return;

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
  }

  void ImGuiManager::processEvent(SDL_Event& event) {
    if (!s_initialized) return;

    ImGui_ImplSDL2_ProcessEvent(&event);
  }

} // namespace JAGEngine
