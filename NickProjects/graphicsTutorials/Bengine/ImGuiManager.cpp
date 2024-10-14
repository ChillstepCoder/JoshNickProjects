#include "ImGuiManager.h"
#include "Window.h"
#include "imgui.h"
#include "ImGui/backends/imgui_impl_sdl2.h"
#include "ImGui/backends/imgui_impl_opengl3.h"
#include <stdio.h>
#include <SDL/SDL.h>
#include <iostream>
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <SDL_opengles2.h>
#else
#include <SDL/SDL_opengl.h>
#endif

namespace Bengine {

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

        SDL_GL_DeleteContext(SDL_GL_GetCurrentContext());
        SDL_Quit();
    }

    void ImGuiManager::newFrame() {
        if (!s_initialized) return;

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();
    }

    void ImGuiManager::renderFrame() {
        ImGui::Render();

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    void ImGuiManager::processEvent(SDL_Event& event) {
        if (!s_initialized) return;
        ImGui_ImplSDL2_ProcessEvent(&event);  
    }
}