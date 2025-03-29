//DialogueApp.cpp

#include "DialogueApp.h"
#include "DialogueSystem.h"
#include "DialogueEditor.h"
#include "DialogueAudioEngine.h"
#include <iostream>
#include <JAGEngine/Window.h>
#include <JAGEngine/InputManager.h>
#include <SDL/SDL.h>
#include <GL/glew.h>
#include <ImGui/imgui.h>
#include <ImGui/imgui_impl_sdl2.h>
#include <ImGui/imgui_impl_opengl3.h>

DialogueApp::DialogueApp() : IMainGame() {
    std::cout << "DialogueApp constructor start\n";
}

DialogueApp::~DialogueApp() {
    std::cout << "DialogueApp destructor\n";
    signalCleanup();
}

void DialogueApp::onInit() {
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

void DialogueApp::addScreens() {
    // No screens to add - we're managing it directly
}

void DialogueApp::onExit() {
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

void DialogueApp::updateAudio() {
    if (m_audioEngine) {
        m_audioEngine->update();
    }
}

void DialogueApp::update() {
    // Call the base implementation first
    IMainGame::update();

    // Update our dialogue editor
    if (m_dialogueEditor) {
        m_dialogueEditor->update();
    }
}

void DialogueApp::draw() {
    // Clear the screen
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(0.45f, 0.55f, 0.60f, 1.00f);

    // Start ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    // Draw the dialogue editor UI
    if (m_dialogueEditor) {
        m_dialogueEditor->render();
    }

    // Our own additional ImGui windows for demonstration purposes
    drawDialogueDemo();

    // End ImGui frame
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void DialogueApp::drawDialogueDemo() {
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
                // Use a placeholder event ID, replace with your actual sound ID
                m_audioEngine->playUISound(12345);
            }
        }

        ImGui::Separator();

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
