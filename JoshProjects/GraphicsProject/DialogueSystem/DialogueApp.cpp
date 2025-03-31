//DialogueApp.cpp

#include "DialogueApp.h"
#include "DialogueScreen.h"
#include <iostream>
#include <JAGEngine/Window.h>
#include <JAGEngine/InputManager.h>
#include <JAGEngine/ScreenList.h>
#include <SDL/SDL.h>

DialogueApp::DialogueApp() : IMainGame() {
    std::cout << "DialogueApp constructor start\n";
    SDL_SetHint("SDL_VIDEO_FULLSCREEN_DISABLED", "1");

    // After IMainGame has set up SDL, initialize ImGui directly
    SDL_Init(SDL_INIT_EVERYTHING);
}


DialogueApp::~DialogueApp() {
    std::cout << "DialogueApp destructor\n";
    signalCleanup();
}

void DialogueApp::onInit() {
    std::cout << "DialogueApp::onInit() start\n";

    // Only initialize ImGui if it's not already initialized
    if (ImGui::GetCurrentContext() == nullptr) {
        initializeImGui();
    }
    else {
        std::cout << "ImGui already initialized by base class" << std::endl;

        // Even if already initialized, set up the style you want
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        ImGui::StyleColorsDark();
    }

    std::cout << "DialogueApp::onInit() complete\n";
}

void DialogueApp::addScreens() {
    std::cout << "DialogueApp::addScreens() - Adding DialogueScreen\n";
    m_dialogueScreen = std::make_unique<DialogueScreen>();

    if (m_screenList) {
        m_screenList->addScreen(m_dialogueScreen.get());
        m_screenList->setScreen(0);

        std::cout << "Added screen to list and set as current\n";
    }
    else {
        std::cerr << "Error: m_screenList is null" << std::endl;
    }
}

void DialogueApp::onExit() {
    std::cout << "DialogueApp::onExit() start\n";
    // The screen will handle cleanup of its own resources
    std::cout << "DialogueApp::onExit() complete\n";
}

void DialogueApp::updateAudio() {
    // No direct audio updates - the screen will handle it
}

void DialogueApp::run() {
    // Initialize
    if (!init()) return;

    std::cout << "Starting custom game loop...\n";

    // Our own game loop
    bool running = true;
    while (running) {
        // Process events with our direct handler
        running = handleEvents();

        // Update
        update();

        // Clear and draw
        draw();

        // Swap buffers
        getWindow().swapBuffer();
    }

    // Cleanup when exiting
    signalCleanup();
}

void DialogueApp::update() {
    static int frameCount = 0;
    frameCount++;

    // Check ImGui input every 60 frames
    if (frameCount % 60 == 0) {
        checkImGuiInput();
    }

    // Rest of your update logic
    IMainGame::update();
}

void DialogueApp::draw() {
    // Clear screen
    glClearColor(0.2f, 0.2f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Start ImGui frame directly
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    // Check if we have a dialogue editor in our screen and use it
    if (m_dialogueScreen && m_dialogueScreen->getDialogueEditor()) {
        m_dialogueScreen->getDialogueEditor()->render();
    }
    else {
        // Fallback if dialogue editor isn't available
        ImGui::SetNextWindowPos(ImVec2(50, 50), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(400, 300), ImGuiCond_FirstUseEver);

        if (ImGui::Begin("Dialogue System")) {
            ImGui::Text("Dialogue editor not available");
        }
        ImGui::End();
    }

    // Render ImGui directly
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

bool DialogueApp::initSystems() {
    // Create window first
    m_window.create("Dialogue System", 1280, 720, 0); // No fullscreen flag

    // Initialize ImGui right after creating window
    initializeImGui();

    // Rest of initialization
    return true;
}

void DialogueApp::checkImGuiInput() {
    ImGuiIO& io = ImGui::GetIO();

    // Check mouse position and buttons
    std::cout << "ImGui Mouse Pos: " << io.MousePos.x << ", " << io.MousePos.y << std::endl;
    std::cout << "ImGui Mouse Down: "
        << (io.MouseDown[0] ? "Left " : "")
        << (io.MouseDown[1] ? "Right " : "")
        << (io.MouseDown[2] ? "Middle" : "")
        << std::endl;
}

bool DialogueApp::handleEvents() {
    // Poll and handle events directly
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        // Process with ImGui first - CRITICAL
        ImGui_ImplSDL2_ProcessEvent(&event);

        // Debug output
        if (event.type == SDL_MOUSEBUTTONDOWN) {
            std::cout << "DIRECT EVENT HANDLER: Mouse button " << (int)event.button.button
                << " at (" << event.button.x << "," << event.button.y << ")" << std::endl;

            // Get ImGui state
            ImGuiIO& io = ImGui::GetIO();
            std::cout << "  ImGui WantCaptureMouse: " << (io.WantCaptureMouse ? "YES" : "NO") << std::endl;
        }

        // Handle quit events
        if (event.type == SDL_QUIT) {
            return false;
        }

        // Handle escape key to exit
        if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) {
            return false;
        }
    }

    return true;
}

bool DialogueApp::processEvents() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        // Let ImGui process the event first
        ImGui_ImplSDL2_ProcessEvent(&event);

        // Process events for application
        if (event.type == SDL_QUIT) {
            return false; // Signal to stop the application
        }

        // Debug output for mouse clicks
        if (event.type == SDL_MOUSEBUTTONDOWN) {
            std::cout << "Mouse clicked at: " << event.button.x << ", " << event.button.y << std::endl;

            // Check if ImGui wants to capture this mouse click
            ImGuiIO& io = ImGui::GetIO();
            std::cout << "ImGui wants this mouse click: " << (io.WantCaptureMouse ? "yes" : "no") << std::endl;
        }

        // Process keyboard events
        if (event.type == SDL_KEYDOWN) {
            if (event.key.keysym.sym == SDLK_ESCAPE) {
                return false; // Exit on ESC key
            }
        }
    }
    return true; // Continue application
}

void DialogueApp::initializeImGui() {
    SDL_Window* window = getWindow().getSDLWindow();
    SDL_GLContext context = getWindow().getGLContext();

    if (!window || !context) {
        std::cerr << "Cannot initialize ImGui: window or context is null" << std::endl;
        return;
    }

    // Create new ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    // Set style
    ImGui::StyleColorsDark();

    // Initialize backend renderers
    ImGui_ImplSDL2_InitForOpenGL(window, context);
    ImGui_ImplOpenGL3_Init("#version 130");

    std::cout << "ImGui initialized successfully" << std::endl;
}

void DialogueApp::renderDialogueInterface() {
    // Clear screen
    glClearColor(0.2f, 0.2f, 0.25f, 1.00f);  // Dark blue-gray background
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Start ImGui frame - make sure this sequence is correct
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    // Main window with simple test content
    ImGui::SetNextWindowPos(ImVec2(50, 50), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(400, 300), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Test Dialog Window")) {
        ImGui::Text("This is a test dialog window");

        // Test button - ensure to print a message when clicked
        if (ImGui::Button("Click This Test Button")) {
            std::cout << "TEST BUTTON CLICKED!" << std::endl;
        }

        // Add a slider to test interaction
        static float value = 0.5f;
        if (ImGui::SliderFloat("Test Slider", &value, 0.0f, 1.0f)) {
            std::cout << "Slider value changed to: " << value << std::endl;
        }
    }
    ImGui::End();

    // Render ImGui
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void DialogueApp::onSDLEvent(SDL_Event& evnt) {
    // Direct pass to ImGui first - most important part
    ImGui_ImplSDL2_ProcessEvent(&evnt);

    // Debug info
    if (evnt.type == SDL_MOUSEBUTTONDOWN) {
        std::cout << "Mouse button " << (int)evnt.button.button
            << " at (" << evnt.button.x << "," << evnt.button.y << ")" << std::endl;
    }

    // Check if ImGui wants to capture this event
    ImGuiIO& io = ImGui::GetIO();

    // Pass to base class ONLY if ImGui doesn't want to capture it
    if (!io.WantCaptureMouse && !io.WantCaptureKeyboard) {
        IMainGame::onSDLEvent(evnt);
    }
}