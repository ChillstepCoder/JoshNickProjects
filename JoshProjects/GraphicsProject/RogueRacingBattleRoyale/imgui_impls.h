// imgui_impls.h

#pragma once

// Include configuration defines first
#define IMGUI_DEFINE_MATH_OPERATORS

// Then include headers
#include <SDL/SDL.h>
#include <GL/glew.h>
#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_sdl2.h"
#include "ImGui/imgui_impl_opengl3.h"

// Helper functions
inline bool InitImGui(SDL_Window* window, SDL_GLContext gl_context) {
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGui::StyleColorsDark();
  if (!ImGui_ImplSDL2_InitForOpenGL(window, gl_context)) return false;
  if (!ImGui_ImplOpenGL3_Init("#version 130")) return false;
  return true;
}

inline void ShutdownImGui() {
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplSDL2_Shutdown();
  ImGui::DestroyContext();
}

inline void NewImGuiFrame() {
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplSDL2_NewFrame();
  ImGui::NewFrame();
}

inline void RenderImGui() {
  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
