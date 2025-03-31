// DialogueApp.h
#pragma once
#include <memory>
#include <JAGEngine/IMainGame.h>
#include <SDL/SDL.h>

// Forward declarations
class DialogueScreen;

// Using the screen approach like your racing game
class DialogueApp : public JAGEngine::IMainGame {
public:
    DialogueApp();
    ~DialogueApp();

    // Required override for IMainGame
    void onInit() override;
    void addScreens() override;
    void onExit() override;

    // Audio update called by the engine
    void updateAudio() override;

    void run();

    // Override update method to handle our own updates
    void update() override;

    // Override draw method to handle our own drawing
    void draw() override;

private:
    bool initSystems();
    void checkImGuiInput();
    bool handleEvents();
    bool processEvents();
    void initializeImGui();
    void renderDialogueInterface();
    std::unique_ptr<DialogueScreen> m_dialogueScreen;
protected:
    void onSDLEvent(SDL_Event& evnt);
};