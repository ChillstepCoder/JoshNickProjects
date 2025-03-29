//DialogueApp.h

#pragma once

#include <memory>
#include <JAGEngine/IMainGame.h>

// Forward declarations
class DialogueManager;
class DialogueEditor;
class DialogueAudioEngine;

// A simple implementation that doesn't use ScreenList
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

    // Override update method to handle our own updates
    void update() override;

    // Override draw method to handle our own drawing
    void draw() override;

private:
    // Custom method for rendering our dialogue demo
    void drawDialogueDemo();

    std::unique_ptr<DialogueAudioEngine> m_audioEngine;
    std::unique_ptr<DialogueManager> m_dialogueManager;
    std::unique_ptr<DialogueEditor> m_dialogueEditor;
};