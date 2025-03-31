//// standalone_main.cpp
//#include <iostream>
//#include <SDL/SDL.h>
//#include <GL/glew.h>
//#include <ImGui/imgui.h>
//#include <ImGui/imgui_impl_sdl2.h>
//#include <ImGui/imgui_impl_opengl3.h>
//#include "DialogueSystem.h"
//#include "DialogueEditor.h"
//
//int main(int argc, char** argv) {
//    std::cout << "Starting standalone dialogue editor...\n";
//
//    // Initialize SDL
//    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
//        std::cerr << "Error: " << SDL_GetError() << std::endl;
//        return -1;
//    }
//
//    // Setup window
//    SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengl");
//    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
//    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
//    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
//    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
//    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
//    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
//    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
//
//    SDL_Window* window = SDL_CreateWindow(
//        "Dialogue System Editor",
//        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
//        1280, 720,
//        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI
//    );
//    if (!window) {
//        std::cerr << "Error creating window: " << SDL_GetError() << std::endl;
//        SDL_Quit();
//        return -1;
//    }
//
//    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
//    SDL_GL_MakeCurrent(window, gl_context);
//    SDL_GL_SetSwapInterval(1); // Enable vsync
//
//    // Initialize GLEW
//    GLenum err = glewInit();
//    if (err != GLEW_OK) {
//        std::cerr << "GLEW initialization failed: " << glewGetErrorString(err) << std::endl;
//        SDL_GL_DeleteContext(gl_context);
//        SDL_DestroyWindow(window);
//        SDL_Quit();
//        return -1;
//    }
//
//    // Initialize ImGui
//    IMGUI_CHECKVERSION();
//    ImGui::CreateContext();
//    ImGuiIO& io = ImGui::GetIO();
//    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable keyboard controls
//    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable gamepad controls
//
//    ImGui::StyleColorsDark();
//    ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
//    ImGui_ImplOpenGL3_Init("#version 130");
//
//    // Initialize dialogue system
//    DialogueManager dialogueManager;
//    dialogueManager.initialize(nullptr); // No audio for now
//
//    // Create some default responses
//    dialogueManager.createResponse(ResponseType::EnthusiasticAffirmative, "Absolutely!");
//    dialogueManager.createResponse(ResponseType::IndifferentAffirmative, "Yeah, sure.");
//    dialogueManager.createResponse(ResponseType::ReluctantAffirmative, "I guess I can do that...");
//    // Add more responses as needed
//
//    DialogueEditor dialogueEditor(&dialogueManager);
//    dialogueEditor.initialize();
//
//    // Main loop
//    bool done = false;
//    while (!done) {
//        // Poll and handle events
//        SDL_Event event;
//        while (SDL_PollEvent(&event)) {
//            ImGui_ImplSDL2_ProcessEvent(&event);
//            if (event.type == SDL_QUIT)
//                done = true;
//            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window))
//                done = true;
//
//            // Debug output for clicks
//            if (event.type == SDL_MOUSEBUTTONDOWN) {
//                std::cout << "Mouse clicked at: " << event.button.x << ", " << event.button.y
//                    << " Button: " << (int)event.button.button << std::endl;
//            }
//        }
//
//        // Start the ImGui frame
//        ImGui_ImplOpenGL3_NewFrame();
//        ImGui_ImplSDL2_NewFrame();
//        ImGui::NewFrame();
//
//        // Render dialogue editor
//        dialogueEditor.render();
//
//        // Rendering
//        ImGui::Render();
//        glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
//        glClearColor(0.25f, 0.30f, 0.35f, 1.00f);
//        glClear(GL_COLOR_BUFFER_BIT);
//        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
//        SDL_GL_SwapWindow(window);
//    }
//
//    // Cleanup
//    ImGui_ImplOpenGL3_Shutdown();
//    ImGui_ImplSDL2_Shutdown();
//    ImGui::DestroyContext();
//
//    SDL_GL_DeleteContext(gl_context);
//    SDL_DestroyWindow(window);
//    SDL_Quit();
//
//    return 0;
//}