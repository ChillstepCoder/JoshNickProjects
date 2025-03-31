// DialogueScreen.h
#pragma once
#include <JAGEngine/IGameScreen.h>
#include <iostream>
#include <memory>
#include <ImGui/imgui.h>
#include "DialogueSystem.h"
#include "DialogueEditor.h"
#include "DialogueAudioEngine.h"

class DialogueScreen : public JAGEngine::IGameScreen {
public:
    DialogueScreen() {
        std::cout << "DialogueScreen constructor\n";
    }

    ~DialogueScreen() {
        std::cout << "DialogueScreen destructor\n";
    }

    // IGameScreen interface
    virtual void build() override {
        std::cout << "DialogueScreen::build\n";

        // Initialize audio engine
        m_audioEngine = std::make_unique<DialogueAudioEngine>();
        if (!m_audioEngine->init("DialogueDemo")) {
            std::cout << "Failed to initialize DialogueAudioEngine!\n";
        }
        else {
            std::cout << "DialogueAudioEngine initialized successfully!\n";
        }

        // Initialize dialogue manager
        m_dialogueManager = std::make_unique<DialogueManager>();
        m_dialogueManager->initialize(m_audioEngine->getWWiseEngine());

        // Initialize dialogue editor
        m_dialogueEditor = std::make_unique<DialogueEditor>(m_dialogueManager.get());
        m_dialogueEditor->initialize();
    }

    virtual void destroy() override {
        std::cout << "DialogueScreen::destroy\n";

        if (m_dialogueEditor) {
            m_dialogueEditor->shutdown();
        }

        if (m_dialogueManager) {
            m_dialogueManager->shutdown();
        }

        if (m_audioEngine) {
            m_audioEngine->cleanup();
        }
    }

    virtual void onEntry() override {
        std::cout << "DialogueScreen::onEntry\n";

        if (m_game) {
            SDL_Window* window = m_game->getWindow().getSDLWindow();
            if (window) {
                // Update the window title
                SDL_SetWindowTitle(window, "Dialogue System");

                SDL_SetWindowFullscreen(window, 0);
                SDL_SetWindowSize(window, 1280, 720);
                SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);

                // Check ImGui initialization
                if (ImGui::GetCurrentContext()) {
                    std::cout << "ImGui context exists in DialogueScreen::onEntry" << std::endl;
                }
                else {
                    std::cout << "Warning: ImGui context does not exist in DialogueScreen::onEntry" << std::endl;
                }
            }
        }
    }

    virtual void onExit() override {
        std::cout << "DialogueScreen::onExit\n";
    }

    virtual void update() override {
        if (m_dialogueEditor) {
            m_dialogueEditor->update();
        }
    }

    virtual void draw() override {
        // Clear the background
        glClearColor(0.2f, 0.2f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Set up ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        // Render dialogue editor
        if (m_dialogueEditor) {
            m_dialogueEditor->render();
        }

        // Finish ImGui frame
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    void handleEvents(SDL_Event& event) {
        // Make sure ImGui processes events
        ImGui_ImplSDL2_ProcessEvent(&event);
    }

    virtual int getNextScreenIndex() const override {
        return -1; // No next screen
    }

    virtual int getPreviousScreenIndex() const override {
        return -1; // No previous screen
    }

    DialogueEditor* getDialogueEditor() const { return m_dialogueEditor.get(); }

private:
    void drawDialogueDemo() {
        ImGui::SetNextWindowPos(ImVec2(50, 50), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(400, 300), ImGuiCond_FirstUseEver);

        if (ImGui::Begin("Dialogue System Demo")) {
            ImGui::Text("This is a simplified dialogue system demo.");
            ImGui::Text("It demonstrates the core functionality without external dependencies.");

            // Display the audio system information
            if (ImGui::CollapsingHeader("Audio System Info")) {
                ImGui::Text("Using DialogueAudioEngine");

                // Volume controls
                float masterVolume = m_audioEngine->getMasterVolume();
                if (ImGui::SliderFloat("Master Volume", &masterVolume, 0.0f, 1.0f)) {
                    m_audioEngine->setMasterVolume(masterVolume);
                }

                float voiceVolume = m_audioEngine->getVoiceVolume();
                if (ImGui::SliderFloat("Voice Volume", &voiceVolume, 0.0f, 1.0f)) {
                    m_audioEngine->setVoiceVolume(voiceVolume);
                }

                float effectsVolume = m_audioEngine->getEffectsVolume();
                if (ImGui::SliderFloat("Effects Volume", &effectsVolume, 0.0f, 1.0f)) {
                    m_audioEngine->setEffectsVolume(effectsVolume);
                }

                if (ImGui::Button("Play UI Sound")) {
                    // Use a placeholder event ID
                    m_audioEngine->playUISound(1234);
                }
            }

            // Display available personalities
            if (ImGui::CollapsingHeader("Available Personalities")) {
                ImGui::Text("Bubbly");
                ImGui::Text("Grumpy");
                ImGui::Text("Manic");
                ImGui::Text("Shy");
                ImGui::Text("Serious");
                ImGui::Text("Anxious");
                ImGui::Text("Confident");
                ImGui::Text("Intellectual");
                ImGui::Text("Mysterious");
                ImGui::Text("Friendly");
            }

            // Display available response types
            if (ImGui::CollapsingHeader("Available Response Types")) {
                ImGui::Text("Enthusiastic Affirmative");
                ImGui::Text("Indifferent Affirmative");
                ImGui::Text("Reluctant Affirmative");
                ImGui::Text("Enthusiastic Negative");
                ImGui::Text("Indifferent Negative");
                ImGui::Text("Untrustworthy Response");
                ImGui::Text("Confused");
                ImGui::Text("Greeting");
                ImGui::Text("Farewell");
                ImGui::Text("Mark on Map");
                ImGui::Text("Give Directions");
                ImGui::Text("Ask for Money");
                ImGui::Text("Offer Help");
                ImGui::Text("Refuse Help");
                ImGui::Text("Ask to Follow");
            }

            ImGui::Separator();

            // Simple demo buttons
            if (ImGui::Button("Generate Text Variants")) {
                m_dialogueManager->generateAllTextVariants();
            }

            ImGui::SameLine();

            if (ImGui::Button("Generate Voice Files")) {
                m_dialogueManager->generateAllVoiceVariants(VoiceType::Male1);
            }

            ImGui::Separator();

            // Show a sample dialogue
            ImGui::Text("Sample Dialogue Flow:");
            ImGui::Text("1. Player: \"Do you know where...\"");
            ImGui::Text("2. Player: \"John is?\"");
            ImGui::Text("3. NPC: [Enthusiastic Affirmative] \"Absolutely!\"");
            ImGui::Text("4. Player: \"Do you like me?\"");
            ImGui::Text("5. Player: \"Yes\"");
            ImGui::Text("6. NPC: [Mark on Map] \"Let me mark him on your map.\"");
        }
        ImGui::End();
    }

    std::unique_ptr<DialogueAudioEngine> m_audioEngine;
    std::unique_ptr<DialogueManager> m_dialogueManager;
    std::unique_ptr<DialogueEditor> m_dialogueEditor;
};