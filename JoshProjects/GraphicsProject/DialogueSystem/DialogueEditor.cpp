//DialogueEditor.cpp
#include "DialogueEditor.h"
#include <ImGui/imgui.h>
#include <ImGui/imgui_stdlib.h>
#include <fstream>
#include <sstream>
#include <direct.h> // For _mkdir
#include <future>
#include <chrono>
#include <thread>
#include <algorithm> // For std::find, std::remove
#include <iostream>

// Custom filesystem namespace for directory creation
namespace fs {
    inline bool create_directories(const std::string& path) {
        // Simple Windows-specific implementation
        return _mkdir(path.c_str()) == 0 || errno == EEXIST;
    }
}

// ==========================================================
// DialogueEditor Implementation
// ==========================================================

DialogueEditor::DialogueEditor(DialogueManager* manager)
    : m_dialogueManager(manager),
    m_activeResponseType(ResponseType::EnthusiasticAffirmative),
    m_selectedPersonality(PersonalityType::Bubbly),
    m_selectedVoice(VoiceType::Male1),
    m_showBatchGenerationWindow(false) {

    // Make sure the audio directory exists
    _mkdir("Audio"); // Create parent directory first
    fs::create_directories("Audio/Dialogue");
}

DialogueEditor::~DialogueEditor() {
    shutdown();
}

void DialogueEditor::initialize() {
    // Load any saved dialogue data
    loadSavedDialogue();

    // Create some default responses if none exist
    if (m_dialogueManager->getAllResponses().empty()) {
        m_dialogueManager->createResponse(ResponseType::EnthusiasticAffirmative, "Absolutely!");
        m_dialogueManager->createResponse(ResponseType::IndifferentAffirmative, "Yeah, sure.");
        m_dialogueManager->createResponse(ResponseType::ReluctantAffirmative, "I guess I can do that...");
        m_dialogueManager->createResponse(ResponseType::EnthusiasticNegative, "No way!");
        m_dialogueManager->createResponse(ResponseType::IndifferentNegative, "Nah.");
        m_dialogueManager->createResponse(ResponseType::UntrustworthyResponse, "I definitely won't tell anyone...");
        m_dialogueManager->createResponse(ResponseType::Confused, "Wait, what?");
        m_dialogueManager->createResponse(ResponseType::Greeting, "Hello there!");
        m_dialogueManager->createResponse(ResponseType::Farewell, "See you around!");
        m_dialogueManager->createResponse(ResponseType::MarkOnMap, "Let me mark that on your map.");
        m_dialogueManager->createResponse(ResponseType::GiveDirections, "Head north past the old oak tree.");
        m_dialogueManager->createResponse(ResponseType::AskForMoney, "Got any spare coins?");
        m_dialogueManager->createResponse(ResponseType::OfferHelp, "I can help you with that.");
        m_dialogueManager->createResponse(ResponseType::RefuseHelp, "Sorry, I can't help with that.");
        m_dialogueManager->createResponse(ResponseType::AskToFollow, "Will you follow me?");
    }
}

void DialogueEditor::shutdown() {
    // Save the dialogue data
    saveDialogue();
}

void DialogueEditor::update() {
    // This could be used for any background processing or pending API calls
    // Get the input manager from the game and process inputs
    if (m_inputManager != nullptr && m_inputManager != reinterpret_cast<JAGEngine::InputManager*>(0x40)) {
        processInput();
    }
}

void DialogueEditor::render() {
    renderMenuBar();
    // Main menu bar
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Save")) {
                saveDialogue();
            }
            if (ImGui::MenuItem("Load")) {
                loadSavedDialogue();
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Exit")) {
                // Handle exit - you'll need to integrate this with your main loop
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("API Settings")) {
            if (ImGui::MenuItem("Configure")) {
                renderAPISettingsWindow();
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Generate")) {
            if (ImGui::MenuItem("Batch Generate Text")) {
                m_showBatchGenerationWindow = true;
                m_batchGenerationTypes.clear();
                m_batchGenerationPersonalities.clear();

                // Add all response types by default
                for (int i = 0; i <= static_cast<int>(ResponseType::AskToFollow); i++) {
                    m_batchGenerationTypes.push_back(static_cast<ResponseType>(i));
                }

                // Add all personality types by default
                for (int i = 0; i <= static_cast<int>(PersonalityType::Friendly); i++) {
                    m_batchGenerationPersonalities.push_back(static_cast<PersonalityType>(i));
                }
            }
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }

    // Response editor window
    renderResponseEditor();

    // Tree editor window (for dialogue trees)
    renderDialogueTreeEditor();

    // Add dialogue navigation test
    ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Dialogue Navigation Test")) {
        testDialogueNavigation();
    }
    ImGui::End();

    // Batch generation window (if active)
    if (m_showBatchGenerationWindow) {
        renderBatchGenerationWindow();
    }
}

void DialogueEditor::setActiveResponse(ResponseType type) {
    m_activeResponseType = type;
}

void DialogueEditor::showBatchGenerationWindow(bool show) {
    m_showBatchGenerationWindow = show;
}

const char* DialogueEditor::getPersonalityName(PersonalityType type) {
    switch (type) {
    case PersonalityType::Bubbly: return "Bubbly";
    case PersonalityType::Grumpy: return "Grumpy";
    case PersonalityType::Manic: return "Manic";
    case PersonalityType::Shy: return "Shy";
    case PersonalityType::Serious: return "Serious";
    case PersonalityType::Anxious: return "Anxious";
    case PersonalityType::Confident: return "Confident";
    case PersonalityType::Intellectual: return "Intellectual";
    case PersonalityType::Mysterious: return "Mysterious";
    case PersonalityType::Friendly: return "Friendly";
    default: return "Unknown";
    }
}

const char* DialogueEditor::getResponseTypeName(ResponseType type) {
    switch (type) {
    case ResponseType::EnthusiasticAffirmative: return "Enthusiastic Affirmative";
    case ResponseType::IndifferentAffirmative: return "Indifferent Affirmative";
    case ResponseType::ReluctantAffirmative: return "Reluctant Affirmative";
    case ResponseType::EnthusiasticNegative: return "Enthusiastic Negative";
    case ResponseType::IndifferentNegative: return "Indifferent Negative";
    case ResponseType::UntrustworthyResponse: return "Untrustworthy Response";
    case ResponseType::Confused: return "Confused";
    case ResponseType::Greeting: return "Greeting";
    case ResponseType::Farewell: return "Farewell";
    case ResponseType::MarkOnMap: return "Mark on Map";
    case ResponseType::GiveDirections: return "Give Directions";
    case ResponseType::AskForMoney: return "Ask for Money";
    case ResponseType::OfferHelp: return "Offer Help";
    case ResponseType::RefuseHelp: return "Refuse Help";
    case ResponseType::AskToFollow: return "Ask to Follow";
    default: return "Unknown";
    }
}

const char* DialogueEditor::getVoiceTypeName(VoiceType type) {
    switch (type) {
    case VoiceType::Male1: return "Male 1";
    case VoiceType::Male2: return "Male 2";
    case VoiceType::Male3: return "Male 3";
    case VoiceType::Female1: return "Female 1";
    case VoiceType::Female2: return "Female 2";
    case VoiceType::Female3: return "Female 3";
    case VoiceType::Child1: return "Child 1";
    case VoiceType::Elderly1: return "Elderly 1";
    case VoiceType::Robot1: return "Robot 1";
    default: return "Unknown";
    }
}

void DialogueEditor::renderResponseEditor() {
    ImGui::SetNextWindowSize(ImVec2(800, 600), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Dialogue Response Editor", nullptr, ImGuiWindowFlags_MenuBar)) {
        if (ImGui::BeginMenuBar()) {
            if (ImGui::BeginMenu("Responses")) {
                // List all response types
                for (int i = 0; i <= static_cast<int>(ResponseType::AskToFollow); i++) {
                    ResponseType type = static_cast<ResponseType>(i);
                    if (ImGui::MenuItem(getResponseTypeName(type), nullptr, m_activeResponseType == type)) {
                        m_activeResponseType = type;
                    }
                }
                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }

        // Get the active response
        auto response = m_dialogueManager->getResponse(m_activeResponseType);
        if (!response) {
            ImGui::Text("Error: Response not found!");
            ImGui::End();
            return;
        }

        // Display response type and name
        ImGui::Text("Response Type: %s", getResponseTypeName(m_activeResponseType));

        std::string responseName = response->getName();
        if (ImGui::InputText("Response Name", &responseName)) {
            response->setName(responseName);
        }

        ImGui::Separator();

        // Personality type selector
        if (ImGui::BeginCombo("Personality", getPersonalityName(m_selectedPersonality))) {
            for (int i = 0; i <= static_cast<int>(PersonalityType::Friendly); i++) {
                PersonalityType type = static_cast<PersonalityType>(i);
                bool isSelected = (m_selectedPersonality == type);
                if (ImGui::Selectable(getPersonalityName(type), isSelected)) {
                    m_selectedPersonality = type;
                }
                if (isSelected) {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }

        // Voice type selector
        if (ImGui::BeginCombo("Voice", getVoiceTypeName(m_selectedVoice))) {
            for (int i = 0; i <= static_cast<int>(VoiceType::Robot1); i++) {
                VoiceType type = static_cast<VoiceType>(i);
                bool isSelected = (m_selectedVoice == type);
                if (ImGui::Selectable(getVoiceTypeName(type), isSelected)) {
                    m_selectedVoice = type;
                }
                if (isSelected) {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }

        ImGui::Separator();

        // Text editor
        std::string text = response->getTextForPersonality(m_selectedPersonality);
        bool isEdited = response->isTextEdited(m_selectedPersonality);

        ImGui::Text("Edit Response Text:");
        if (isEdited) {
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Status: Manually Edited");
        }
        else {
            ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Status: AI Generated (Unedited)");
        }

        if (ImGui::InputTextMultiline("##responseText", &text, ImVec2(-1.0f, 100.0f))) {
            response->setTextForPersonality(m_selectedPersonality, text, true);
        }

        // Generate button
        if (ImGui::Button("Regenerate with GPT")) {
            std::string generatedText = m_dialogueManager->generateTextWithGPT(
                m_activeResponseType, m_selectedPersonality, text);
            response->setTextForPersonality(m_selectedPersonality, generatedText, false);
        }

        ImGui::SameLine();

        // Accept generated button (marks as edited)
        if (!isEdited && ImGui::Button("Accept Generated")) {
            response->setTextForPersonality(m_selectedPersonality, text, true);
        }

        ImGui::Separator();

        // Voice generation
        ImGui::Text("Voice Generation:");

        bool hasVoice = response->hasVoiceGenerated(m_selectedPersonality, m_selectedVoice);
        if (hasVoice) {
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Voice Generated");
            std::string voicePath = response->getVoiceFilePath(m_selectedPersonality, m_selectedVoice);
            ImGui::Text("Path: %s", voicePath.c_str());

            if (ImGui::Button("Play")) {
                // Integrate with your audio engine to play the file
            }

            ImGui::SameLine();

            if (ImGui::Button("Regenerate Voice")) {
                // Generate output path
                std::string responseType = std::to_string(static_cast<int>(m_activeResponseType));
                std::string personalityType = std::to_string(static_cast<int>(m_selectedPersonality));
                std::string voiceType = std::to_string(static_cast<int>(m_selectedVoice));

                std::string outputPath = "Audio/Dialogue/";
                outputPath += "response" + responseType + "_";
                outputPath += "personality" + personalityType + "_";
                outputPath += "voice" + voiceType + ".mp3";

                if (m_dialogueManager->generateVoiceWithElevenLabs(text, m_selectedPersonality, m_selectedVoice, outputPath)) {
                    response->setVoiceFilePath(m_selectedPersonality, m_selectedVoice, outputPath);
                }
            }
        }
        else {
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "No Voice Generated");

            if (ImGui::Button("Generate Voice")) {
                // Generate output path
                std::string responseType = std::to_string(static_cast<int>(m_activeResponseType));
                std::string personalityType = std::to_string(static_cast<int>(m_selectedPersonality));
                std::string voiceType = std::to_string(static_cast<int>(m_selectedVoice));

                std::string outputPath = "Audio/Dialogue/";
                outputPath += "response" + responseType + "_";
                outputPath += "personality" + personalityType + "_";
                outputPath += "voice" + voiceType + ".mp3";

                if (m_dialogueManager->generateVoiceWithElevenLabs(text, m_selectedPersonality, m_selectedVoice, outputPath)) {
                    response->setVoiceFilePath(m_selectedPersonality, m_selectedVoice, outputPath);
                }
            }
        }
    }
    ImGui::End();
}

void DialogueEditor::renderDialogueTreeEditor() {
    ImGui::SetNextWindowSize(ImVec2(800, 400), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Dialogue Tree Editor")) {
        // Simplified button row with clearer distinctions
        if (ImGui::Button("Add NPC Statement")) {
            auto newNode = m_dialogueManager->createDialogueNode(
                DialogueNode::NodeType::NPCStatement,
                "New NPC Statement"
            );

            // Check if there's a selected node and add the new node as its child
            if (m_selectedNodeId >= 0) {
                auto selectedNode = m_dialogueManager->getNodeById(m_selectedNodeId);
                if (selectedNode) {
                    selectedNode->addChildNode(newNode, "");
                }
            }
            // If root or nothing is selected (-1), we leave it as a standalone root node

            m_selectedNodeId = newNode->getId();
        }

        ImGui::SameLine();

        if (ImGui::Button("Add Player Choice")) {
            auto newNode = m_dialogueManager->createDialogueNode(
                DialogueNode::NodeType::PlayerChoice,
                "New Player Choice"
            );

            // Check if there's a selected node and add the new node as its child
            if (m_selectedNodeId >= 0) {
                auto selectedNode = m_dialogueManager->getNodeById(m_selectedNodeId);
                if (selectedNode) {
                    selectedNode->addChildNode(newNode, "");
                }
            }

            m_selectedNodeId = newNode->getId();
        }

        ImGui::SameLine();

        if (ImGui::Button("Add Condition Check")) {
            auto newNode = m_dialogueManager->createDialogueNode(
                DialogueNode::NodeType::ConditionCheck,
                "New Condition Check"
            );

            // Check if there's a selected node and add the new node as its child
            if (m_selectedNodeId >= 0) {
                auto selectedNode = m_dialogueManager->getNodeById(m_selectedNodeId);
                if (selectedNode) {
                    selectedNode->addChildNode(newNode, "");
                }
            }

            m_selectedNodeId = newNode->getId();
        }

        if (m_dialogueManager->getAllNodes().size() > 0 && ImGui::Button("Delete Selected")) {
            if (m_selectedNodeId >= 0) {
                m_dialogueManager->deleteNode(m_selectedNodeId);
                m_selectedNodeId = -1;
            }
        }

        // Testing tools
        ImGui::SameLine();
        if (ImGui::Button("Create Test Tree")) {
            createRelationshipTestTree();
        }

        ImGui::Separator();

        // Split the window into two panes
        float propertyPaneWidth = ImGui::GetContentRegionAvail().x * 0.4f;

        ImGui::BeginChild("NodeProperties", ImVec2(propertyPaneWidth, 0), true);
        renderNodeProperties();
        ImGui::EndChild();

        ImGui::SameLine();

        ImGui::BeginChild("NodeTree", ImVec2(0, 0), true);
        ImGui::Text("Dialogue Tree Structure:");
        ImGui::Separator();
        renderNodeTree();
        ImGui::EndChild();
    }
    ImGui::End();
}

void DialogueEditor::renderAPISettingsWindow() {
    ImGui::SetNextWindowSize(ImVec2(500, 200), ImGuiCond_FirstUseEver);
    ImGui::Begin("API Settings", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

    static char gptApiKey[256] = "";
    static char elevenLabsApiKey[256] = "";

    ImGui::Text("Enter your API keys to use GPT and ElevenLabs services:");
    ImGui::Separator();

    ImGui::Text("OpenAI GPT API Key:");
    ImGui::InputText("##gptApiKey", gptApiKey, IM_ARRAYSIZE(gptApiKey), ImGuiInputTextFlags_Password);

    ImGui::Text("ElevenLabs API Key:");
    ImGui::InputText("##elevenLabsApiKey", elevenLabsApiKey, IM_ARRAYSIZE(elevenLabsApiKey), ImGuiInputTextFlags_Password);

    ImGui::Separator();

    if (ImGui::Button("Save", ImVec2(120, 0))) {
        m_dialogueManager->setAPIKeys(gptApiKey, elevenLabsApiKey);

        // Save keys to a config file (consider encrypting them in a real app)
        std::ofstream configFile("dialogue_config.ini");
        if (configFile.is_open()) {
            configFile << "[API]\n";
            configFile << "GPTKey=" << gptApiKey << "\n";
            configFile << "ElevenLabsKey=" << elevenLabsApiKey << "\n";
            configFile.close();
        }
    }

    ImGui::SameLine();

    if (ImGui::Button("Load", ImVec2(120, 0))) {
        // Load keys from config file
        std::ifstream configFile("dialogue_config.ini");
        if (configFile.is_open()) {
            std::string line;
            while (std::getline(configFile, line)) {
                size_t delimPos = line.find('=');
                if (delimPos != std::string::npos) {
                    std::string key = line.substr(0, delimPos);
                    std::string value = line.substr(delimPos + 1);

                    if (key == "GPTKey") {
                        strncpy_s(gptApiKey, value.c_str(), sizeof(gptApiKey) - 1);
                        gptApiKey[sizeof(gptApiKey) - 1] = '\0';
                    }
                    else if (key == "ElevenLabsKey") {
                        strncpy_s(elevenLabsApiKey, value.c_str(), sizeof(elevenLabsApiKey) - 1);
                        elevenLabsApiKey[sizeof(elevenLabsApiKey) - 1] = '\0';
                    }
                }
            }
            configFile.close();

            // Set the keys in the manager
            m_dialogueManager->setAPIKeys(gptApiKey, elevenLabsApiKey);
        }
    }

    ImGui::SameLine();

    if (ImGui::Button("Cancel", ImVec2(120, 0))) {
        // Just close the window
    }

    ImGui::End();
}

void DialogueEditor::renderMenuBar() {
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Save")) {
                saveDialogue();
            }
            if (ImGui::MenuItem("Load")) {
                loadSavedDialogue();
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Exit")) {
                // Handle exit - you'll need to integrate this with your main loop
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Generation")) {
            if (ImGui::MenuItem("Generate All Text Variants")) {
                for (auto& response : m_dialogueManager->getAllResponses()) {
                    m_dialogueManager->generatePersonalityVariants(response);
                }
            }
            if (ImGui::MenuItem("Generate All Voice Variants")) {
                m_dialogueManager->generateAllVoiceVariants();
            }
            if (ImGui::MenuItem("Register With Wwise")) {
                m_dialogueManager->registerWithWwise();
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("API Settings")) {
            if (ImGui::MenuItem("Configure APIs")) {
                m_showApiConfigWindow = true;
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Test")) {
            if (ImGui::MenuItem("Test Playback")) {
                // Play a sample dialogue for testing
                m_dialogueManager->playDialogueResponse(
                    ResponseType::EnthusiasticAffirmative,
                    PersonalityType::Bubbly,
                    VoiceType::Male1
                );
            }
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }
}

void DialogueEditor::renderBatchGenerationWindow() {
    ImGui::SetNextWindowSize(ImVec2(600, 400), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Batch Generation", &m_showBatchGenerationWindow)) {
        ImGui::Text("Select what to generate:");
        ImGui::Separator();

        // Response Types selection
        if (ImGui::CollapsingHeader("Response Types", ImGuiTreeNodeFlags_DefaultOpen)) {
            if (ImGui::Button("Select All Responses")) {
                m_batchGenerationTypes.clear();
                for (int i = 0; i <= static_cast<int>(ResponseType::AskToFollow); i++) {
                    m_batchGenerationTypes.push_back(static_cast<ResponseType>(i));
                }
            }

            ImGui::SameLine();

            if (ImGui::Button("Clear Responses")) {
                m_batchGenerationTypes.clear();
            }

            ImGui::Separator();

            for (int i = 0; i <= static_cast<int>(ResponseType::AskToFollow); i++) {
                ResponseType type = static_cast<ResponseType>(i);

                bool isSelected = std::find(m_batchGenerationTypes.begin(), m_batchGenerationTypes.end(), type) != m_batchGenerationTypes.end();
                if (ImGui::Checkbox(getResponseTypeName(type), &isSelected)) {
                    if (isSelected) {
                        m_batchGenerationTypes.push_back(type);
                    }
                    else {
                        m_batchGenerationTypes.erase(
                            std::remove(m_batchGenerationTypes.begin(), m_batchGenerationTypes.end(), type),
                            m_batchGenerationTypes.end());
                    }
                }
            }
        }

        // Personality Types selection
        if (ImGui::CollapsingHeader("Personality Types", ImGuiTreeNodeFlags_DefaultOpen)) {
            if (ImGui::Button("Select All Personalities")) {
                m_batchGenerationPersonalities.clear();
                for (int i = 0; i <= static_cast<int>(PersonalityType::Friendly); i++) {
                    m_batchGenerationPersonalities.push_back(static_cast<PersonalityType>(i));
                }
            }

            ImGui::SameLine();

            if (ImGui::Button("Clear Personalities")) {
                m_batchGenerationPersonalities.clear();
            }

            ImGui::Separator();

            for (int i = 0; i <= static_cast<int>(PersonalityType::Friendly); i++) {
                PersonalityType type = static_cast<PersonalityType>(i);

                bool isSelected = std::find(m_batchGenerationPersonalities.begin(), m_batchGenerationPersonalities.end(), type) != m_batchGenerationPersonalities.end();
                if (ImGui::Checkbox(getPersonalityName(type), &isSelected)) {
                    if (isSelected) {
                        m_batchGenerationPersonalities.push_back(type);
                    }
                    else {
                        m_batchGenerationPersonalities.erase(
                            std::remove(m_batchGenerationPersonalities.begin(), m_batchGenerationPersonalities.end(), type),
                            m_batchGenerationPersonalities.end());
                    }
                }
            }
        }

        // Voice Types selection (for voice generation)
        if (ImGui::CollapsingHeader("Voice Types", ImGuiTreeNodeFlags_DefaultOpen)) {
            if (ImGui::Button("Select All Voices")) {
                m_batchGenerationVoices.clear();
                for (int i = 0; i <= static_cast<int>(VoiceType::Robot1); i++) {
                    m_batchGenerationVoices.push_back(static_cast<VoiceType>(i));
                }
            }

            ImGui::SameLine();

            if (ImGui::Button("Clear Voices")) {
                m_batchGenerationVoices.clear();
            }

            ImGui::Separator();

            for (int i = 0; i <= static_cast<int>(VoiceType::Robot1); i++) {
                VoiceType type = static_cast<VoiceType>(i);

                bool isSelected = std::find(m_batchGenerationVoices.begin(), m_batchGenerationVoices.end(), type) != m_batchGenerationVoices.end();
                if (ImGui::Checkbox(getVoiceTypeName(type), &isSelected)) {
                    if (isSelected) {
                        m_batchGenerationVoices.push_back(type);
                    }
                    else {
                        m_batchGenerationVoices.erase(
                            std::remove(m_batchGenerationVoices.begin(), m_batchGenerationVoices.end(), type),
                            m_batchGenerationVoices.end());
                    }
                }
            }
        }

        ImGui::Separator();

        // Generate buttons
        if (ImGui::Button("Generate Selected Text", ImVec2(200, 0))) {
            // Start a background thread for text generation
            std::thread([this]() {
                for (ResponseType type : m_batchGenerationTypes) {
                    auto response = m_dialogueManager->getResponse(type);
                    if (!response) continue;

                    for (PersonalityType personality : m_batchGenerationPersonalities) {
                        if (!response->isTextEdited(personality)) {
                            std::string defaultText = response->getTextForPersonality(personality);
                            std::string generatedText = m_dialogueManager->generateTextWithGPT(type, personality, defaultText);
                            response->setTextForPersonality(personality, generatedText, false);

                            // Sleep briefly to avoid overwhelming the API
                            std::this_thread::sleep_for(std::chrono::milliseconds(100));
                        }
                    }
                }
                }).detach();
        }

        ImGui::SameLine();

        if (ImGui::Button("Generate Selected Voice", ImVec2(200, 0))) {
            // Start a background thread for voice generation
            std::thread([this]() {
                for (ResponseType type : m_batchGenerationTypes) {
                    auto response = m_dialogueManager->getResponse(type);
                    if (!response) continue;

                    for (PersonalityType personality : m_batchGenerationPersonalities) {
                        std::string text = response->getTextForPersonality(personality);

                        for (VoiceType voice : m_batchGenerationVoices) {
                            // Generate output path
                            std::string responseType = std::to_string(static_cast<int>(type));
                            std::string personalityType = std::to_string(static_cast<int>(personality));
                            std::string voiceType = std::to_string(static_cast<int>(voice));

                            std::string outputPath = "Audio/Dialogue/";
                            outputPath += "response" + responseType + "_";
                            outputPath += "personality" + personalityType + "_";
                            outputPath += "voice" + voiceType + ".mp3";

                            if (m_dialogueManager->generateVoiceWithElevenLabs(text, personality, voice, outputPath)) {
                                response->setVoiceFilePath(personality, voice, outputPath);
                            }

                            // Sleep briefly to avoid overwhelming the API
                            std::this_thread::sleep_for(std::chrono::milliseconds(100));
                        }
                    }
                }
                }).detach();
        }
    }
    ImGui::End();
}

void DialogueEditor::renderNodeTree()
{

    // Get all root nodes (nodes without parents)
    auto allNodes = m_dialogueManager->getAllNodes();
    std::vector<std::shared_ptr<DialogueNode>> rootNodes;
    for (const auto& node : allNodes)
    {
        if (!m_dialogueManager->hasParent(node->getId()))
            rootNodes.push_back(node);
    }

    // If no root nodes, show a message
    if (rootNodes.empty())
    {
        ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f),
            "No dialogue nodes created yet. Use the buttons above to start.");
        return;
    }

    // Recursive lambda to render a node and its children
    std::function<void(std::shared_ptr<DialogueNode>, int)> renderNodeRecursive;
    renderNodeRecursive = [this, &renderNodeRecursive](std::shared_ptr<DialogueNode> node, int depth)
        {
            if (!node) return;

            // Tree flags
            ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
            if (node->getId() == m_selectedNodeId)
                flags |= ImGuiTreeNodeFlags_Selected;

            auto children = node->getChildren();
            if (children.empty())
                flags |= ImGuiTreeNodeFlags_Leaf;

            // Build display text
            std::string nodeText = node->getText();
            if (nodeText.empty())
                nodeText = "Node " + std::to_string(node->getId());

            // Append extra info for ConditionCheck or NPCStatement
            if (node->getType() == DialogueNode::NodeType::ConditionCheck)
            {
                nodeText += " (" + node->getCondition().GetDescription() + ")";
            }
            else if (node->getType() == DialogueNode::NodeType::NPCStatement)
            {
                auto response = node->getResponse();
                if (response)
                    nodeText += " ? " + m_dialogueManager->getResponseTypeName(response->getType());
            }

            // Determine color & type label
            ImVec4 nodeColor;
            std::string typeIndicator;
            switch (node->getType())
            {
            case DialogueNode::NodeType::NPCStatement:
                nodeColor = ImVec4(0.0f, 0.8f, 0.0f, 1.0f);
                typeIndicator = "[NPC]";
                break;
            case DialogueNode::NodeType::PlayerChoice:
                nodeColor = ImVec4(0.8f, 0.8f, 0.0f, 1.0f);
                typeIndicator = "[Player]";
                break;
            case DialogueNode::NodeType::ConditionCheck:
                nodeColor = ImVec4(0.8f, 0.4f, 0.0f, 1.0f);
                typeIndicator = "[Condition]";
                break;
            case DialogueNode::NodeType::BranchPoint:
                nodeColor = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
                typeIndicator = "[Branch]";
                break;
            default:
                nodeColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
                typeIndicator = "[Unknown]";
                break;
            }

            //
            // 1) Render the tree arrow with a unique ID (pointer-based)
            //
            ImGui::PushStyleColor(ImGuiCol_Text, nodeColor);
            // Use the pointer as the ID, pass a trivial label format (non-null).
            bool nodeOpen = ImGui::TreeNodeEx((void*)(intptr_t)node->getId(), flags, "%s", " ");
            ImGui::PopStyleColor();

            // If arrow area clicked, toggle selection (select if not selected, deselect if already selected)
            if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
                if (m_selectedNodeId == node->getId()) {
                    // Deselect the node if it's already selected
                    m_selectedNodeId = -1;
                }
                else {
                    // Otherwise select the node
                    m_selectedNodeId = node->getId();
                }
            }

            // Same for the text area click handler
            if (ImGui::IsItemClicked()) {
                if (m_selectedNodeId == node->getId()) {
                    // Deselect the node if it's already selected
                    m_selectedNodeId = -1;
                }
                else {
                    // Otherwise select the node
                    m_selectedNodeId = node->getId();
                }
            }

            ImGui::SameLine();
            // Push a separate ID for the text so it can be clicked separately
            ImGui::PushID(node->getId());
            {
                // Wrap to the right edge
                float wrapPos = ImGui::GetCursorPos().x + ImGui::GetContentRegionAvail().x;
                ImGui::PushTextWrapPos(wrapPos);

                // Show the text (nodeText + typeIndicator)
                ImGui::TextWrapped("%s %s", nodeText.c_str(), typeIndicator.c_str());

                // If the user clicks on the text, select node
                if (ImGui::IsItemClicked())
                    m_selectedNodeId = node->getId();

                ImGui::PopTextWrapPos();
            }
            ImGui::PopID();

            //
            // 3) Context menu on the arrow area
            //
            if (ImGui::BeginPopupContextItem())
            {
                if (ImGui::BeginMenu("Add Child Node"))
                {
                    if (ImGui::MenuItem("NPC Statement"))
                    {
                        auto childNode = m_dialogueManager->createDialogueNode(
                            DialogueNode::NodeType::NPCStatement, "New NPC Statement");
                        node->addChildNode(childNode);
                    }
                    if (ImGui::MenuItem("Player Choice"))
                    {
                        auto childNode = m_dialogueManager->createDialogueNode(
                            DialogueNode::NodeType::PlayerChoice, "New Player Choice");
                        node->addChildNode(childNode);
                    }
                    if (ImGui::MenuItem("Condition Check"))
                    {
                        auto childNode = m_dialogueManager->createDialogueNode(
                            DialogueNode::NodeType::ConditionCheck, "New Condition Check");
                        node->addChildNode(childNode);
                    }
                    if (ImGui::MenuItem("Branch Point"))
                    {
                        auto childNode = m_dialogueManager->createDialogueNode(
                            DialogueNode::NodeType::BranchPoint, "New Branch Point");
                        node->addChildNode(childNode);
                    }
                    ImGui::EndMenu();
                }

                if (ImGui::MenuItem("Duplicate Node"))
                {
                    auto newNode = m_dialogueManager->duplicateNode(node->getId());
                    if (newNode)
                    {
                        if (m_dialogueManager->hasParent(node->getId()))
                        {
                            auto parent = m_dialogueManager->findParentNode(node->getId());
                            if (parent)
                                parent->addChildNode(newNode, "Duplicated");
                        }
                        m_selectedNodeId = newNode->getId();
                    }
                }

                ImGui::Separator();
                if (ImGui::MenuItem("Delete Node"))
                {
                    m_dialogueManager->deleteNode(node->getId());
                    m_selectedNodeId = -1;
                    ImGui::EndPopup();
                    if (nodeOpen)
                        ImGui::TreePop();
                    return;
                }
                ImGui::EndPopup();
            }

            //
            // 4) Recursively render children if open
            //
            if (nodeOpen)
            {
                if (node->getType() == DialogueNode::NodeType::ConditionCheck)
                {
                    // Color-code child branches
                    int branchIndex = 0;
                    for (auto& childPair : children)
                    {
                        ImVec4 branchColor;
                        if (children.size() == 2)
                        {
                            // Binary condition: green for success, red for failure
                            branchColor = (branchIndex == 0)
                                ? ImVec4(0.0f, 0.8f, 0.0f, 1.0f)
                                : ImVec4(0.8f, 0.0f, 0.0f, 1.0f);
                        }
                        else
                        {
                            // Multi-branch: gradient from green to red
                            float t = static_cast<float>(branchIndex) / (children.size() - 1);
                            branchColor = ImVec4(t * 0.8f, (1.0f - t) * 0.8f, 0.0f, 1.0f);
                        }

                        // Show branch label if any
                        if (!childPair.second.empty())
                            ImGui::TextColored(branchColor, "?? [%s] ??", childPair.second.c_str());

                        // Render child
                        ImGui::PushStyleColor(ImGuiCol_Text, branchColor);
                        renderNodeRecursive(childPair.first, depth + 1);
                        ImGui::PopStyleColor();

                        branchIndex++;
                    }
                }
                else
                {
                    // Normal children
                    for (auto& childPair : children)
                    {
                        if (!childPair.second.empty())
                            ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f),
                                "?? [%s] ??", childPair.second.c_str());
                        renderNodeRecursive(childPair.first, depth + 1);
                    }
                }
                ImGui::TreePop();
            }
        };

        // Display a special "ROOT" node at the top
        ImGuiTreeNodeFlags rootFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
        if (m_selectedNodeId == -1)  // Use -1 as the special ID for "ROOT"
            rootFlags |= ImGuiTreeNodeFlags_Selected;

        // Display the special root node using a unique style
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 1.0f, 1.0f));  // Blue color
        bool rootOpen = ImGui::TreeNodeEx("##root", rootFlags, "  [ROOT]");
        ImGui::PopStyleColor();

        // If the root node is clicked, select it (by setting selected ID to -1)
        if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
            m_selectedNodeId = -1;  // Special ID for root
        }

        if (rootOpen) {
            // Now render all the actual root nodes as children of this special root
            for (auto& rootNode : rootNodes) {
                renderNodeRecursive(rootNode, 0);
            }
            ImGui::TreePop();
        }
}

void DialogueEditor::handleKeyPress(unsigned int keyID) {
    if (keyID == 127) {  // Delete key code
        std::cout << "Delete key handled in DialogueEditor" << std::endl;
        if (m_selectedNodeId >= 0) {
            std::cout << "Deleting node: " << m_selectedNodeId << std::endl;
            m_dialogueManager->deleteNode(m_selectedNodeId);
            m_selectedNodeId = -1;
        }
    }
}

void DialogueEditor::processInput() {
    // Safety check
    if (!m_inputManager) {
        return;
    }

    // Add debugging
    std::cout << "Processing input in DialogueEditor" << std::endl;

    // Check for Delete key with multiple possible key codes
    // Common key codes: SDLK_DELETE (127), SDL_SCANCODE_DELETE (46)
    if (m_inputManager->isKeyPressed(127) || m_inputManager->isKeyPressed(46)) {
        std::cout << "Delete key pressed!" << std::endl;
        if (m_selectedNodeId >= 0) {
            m_dialogueManager->deleteNode(m_selectedNodeId);
            m_selectedNodeId = -1;
        }
    }
}


void DialogueEditor::updateInventoryBranchLabel(std::shared_ptr<DialogueNode> node, int childId, const BranchCondition& branchCond) {
    if (!node) return;

    std::string itemId = branchCond.itemId;
    int minQty = branchCond.minQuantity;
    int maxQty = branchCond.maxQuantity;

    // Generate appropriate label based on quantity range
    std::string label = "Has";
    if (minQty == 0 && maxQty == 0) {
        label = "No";
    }
    else if (minQty == maxQty) {
        label = "Has exactly " + std::to_string(minQty);
    }
    else if (maxQty == 9999) {
        label = "Has " + std::to_string(minQty) + "+";
    }
    else {
        label = "Has " + std::to_string(minQty) + "-" + std::to_string(maxQty);
    }

    // Update the child condition with item info
    node->updateChildCondition(childId, label + " " + itemId);
}

// Helper function to update stat branch labels
void DialogueEditor::updateStatBranchLabel(std::shared_ptr<DialogueNode> node, int childId, const BranchCondition& branchCond) {
    if (!node) return;

    std::string statName = branchCond.statName;
    int minVal = branchCond.minValue;
    int maxVal = branchCond.maxValue;

    // Generate appropriate label based on value range
    std::string label;
    if (minVal == maxVal) {
        label = statName + " = " + std::to_string(minVal);
    }
    else if (maxVal == 9999) {
        label = statName + " ? " + std::to_string(minVal);
    }
    else if (minVal == 0) {
        label = statName + " ? " + std::to_string(maxVal);
    }
    else {
        label = statName + ": " + std::to_string(minVal) + "-" + std::to_string(maxVal);
    }

    // Update the child condition
    node->updateChildCondition(childId, label);
}

void DialogueEditor::renderNodeProperties() {
    // Enable dynamic text wrapping for all UI elements in this window.
    ImGui::PushTextWrapPos(0.0f);

    // Only show properties if a node is selected.
    if (m_selectedNodeId < 0) {
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Select a node to edit its properties");
        ImGui::PopTextWrapPos();
        return;
    }

    auto node = m_dialogueManager->getNodeById(m_selectedNodeId);
    if (!node) {
        m_selectedNodeId = -1; // Reset selection if node doesn't exist.
        ImGui::PopTextWrapPos();
        return;
    }

    ImGui::TextWrapped("Node Properties (ID: %d)", m_selectedNodeId);

    // Node type selector with dynamic width.
    const char* nodeTypes[] = {
        "NPC Statement",
        "Player Choice",
        "Condition Check",
        "Branch Point"
    };
    int currentType = static_cast<int>(node->getType());
    float dropdownWidth = ImGui::GetContentRegionAvail().x * 0.95f;
    ImGui::TextUnformatted("Node Type:");
    ImGui::SameLine();

    // Decide on a smaller width for the combo—say 40% of the available region
    float comboWidth = ImGui::GetContentRegionAvail().x * 0.80f;
    ImGui::PushItemWidth(comboWidth);

    // Now use a label that’s hidden (##NodeType) so the preceding text is the visible label
    if (ImGui::Combo("##NodeType", &currentType, nodeTypes, IM_ARRAYSIZE(nodeTypes))) {
        m_dialogueManager->updateNodeType(m_selectedNodeId, static_cast<DialogueNode::NodeType>(currentType));
    }

    ImGui::PopItemWidth();

    // Node text editor.
    std::string nodeText = node->getText();
    ImGui::TextWrapped("Node Text:");
    if (node->getType() == DialogueNode::NodeType::PlayerChoice) {
        ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.0f, 1.0f), "This is what the player will select");
    }
    else if (node->getType() == DialogueNode::NodeType::NPCStatement) {
        ImGui::TextColored(ImVec4(0.0f, 0.8f, 0.0f, 1.0f), "This is what the NPC will say");
    }
    if (ImGui::InputTextMultiline("##NodeText", &nodeText, ImVec2(-FLT_MIN, 60.0f))) {
        node->setText(nodeText);
    }

    // Response selector (only for NPC statements)
    if (node->getType() == DialogueNode::NodeType::NPCStatement) {
        ImGui::Separator();
        ImGui::TextWrapped("Associated Response Type:");

        // Get all response types for selection.
        std::vector<ResponseType> responseTypes;
        for (int i = 0; i <= static_cast<int>(ResponseType::AskToFollow); i++) {
            responseTypes.push_back(static_cast<ResponseType>(i));
        }

        auto currentResponse = node->getResponse();
        ResponseType selectedType = currentResponse ? currentResponse->getType() : ResponseType::EnthusiasticAffirmative;
        std::string selectedTypeName = m_dialogueManager->getResponseTypeName(selectedType);
        if (ImGui::BeginCombo("Response Type", selectedTypeName.c_str())) {
            for (ResponseType type : responseTypes) {
                bool isSelected = (selectedType == type);
                std::string typeName = m_dialogueManager->getResponseTypeName(type);
                if (ImGui::Selectable(typeName.c_str(), isSelected)) {
                    auto response = m_dialogueManager->getResponse(type);
                    if (response) {
                        node->setResponse(response);
                    }
                }
                if (isSelected) {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }

        ImGui::TextWrapped("Personality & Voice Options:");
        if (currentResponse) {
            // Helper UI for testing different personalities.
            static int currentPersonality = static_cast<int>(PersonalityType::Bubbly);
            const char* personalities[] = {
                "Bubbly", "Grumpy", "Manic", "Shy", "Serious",
                "Anxious", "Confident", "Intellectual", "Mysterious", "Friendly"
            };
            float personalityDropdownWidth = ImGui::GetContentRegionAvail().x * 0.95f;
            ImGui::PushItemWidth(personalityDropdownWidth);
            if (ImGui::Combo("Test Personality", &currentPersonality, personalities, IM_ARRAYSIZE(personalities))) {
                // Update UI only.
            }
            ImGui::PopItemWidth();

            PersonalityType personality = static_cast<PersonalityType>(currentPersonality);
            std::string responseText = currentResponse->getTextForPersonality(personality);
            ImGui::TextColored(ImVec4(0.0f, 0.8f, 0.0f, 1.0f), "Response text: \"%s\"", responseText.c_str());
        }
    }

    // Enhanced condition editor for condition nodes.
    if (node->getType() == DialogueNode::NodeType::ConditionCheck) {
        ImGui::Separator();
        ImGui::TextWrapped("Condition Properties:");

        DialogueCondition condition = node->getCondition();

        // Condition type selector.
        ImGui::TextUnformatted("Condition Type:");
        ImGui::SameLine();
        float condDropdownWidth = ImGui::GetContentRegionAvail().x * 0.80f;
        ImGui::PushItemWidth(condDropdownWidth);
        const char* conditionTypes[] = {
            "None",
            "Relationship Status",
            "Quest Status",
            "Inventory Check",
            "Stat Check",
            "Time Of Day",
            "Player Choice",
            "Custom"
        };
        int currentCondType = static_cast<int>(condition.type);
        bool conditionTypeChanged = false;
        if (ImGui::Combo("##ConditionType", &currentCondType, conditionTypes, IM_ARRAYSIZE(conditionTypes))) {
            condition.type = static_cast<ConditionType>(currentCondType);
            node->setCondition(condition);
            conditionTypeChanged = true;
        }
        ImGui::PopItemWidth();

        // Condition-specific parameters.
        switch (condition.type) {
        case ConditionType::RelationshipStatus: {
            const char* relationshipLevels[] = { "Hostile", "Unfriendly", "Neutral", "Friendly", "Close" };
            int currentLevel = static_cast<int>(condition.relationshipThreshold);
            bool changed = false;

            ImGui::TextUnformatted("Minimum Relationship Level:");
            ImGui::SameLine();
            float relDropdownWidth = ImGui::GetContentRegionAvail().x * 0.80f;
            ImGui::PushItemWidth(relDropdownWidth);
            if (ImGui::Combo("##RelationshipLevel", &currentLevel, relationshipLevels, IM_ARRAYSIZE(relationshipLevels))) {
                condition.relationshipThreshold = static_cast<RelationshipStatus>(currentLevel);
                node->setCondition(condition);
                changed = true;
            }
            ImGui::PopItemWidth();

            ImGui::TextColored(ImVec4(0.0f, 0.8f, 0.8f, 1.0f),
                "This checks if NPC's relationship with player is at least:");
            ImGui::TextColored(ImVec4(0.0f, 0.9f, 0.0f, 1.0f), "  %s", relationshipLevels[currentLevel]);

            ImGui::Separator();
            ImGui::TextColored(ImVec4(0.8f, 0.7f, 0.0f, 1.0f), "Branch outcomes:");
            {
                auto branchLabels = condition.GetBranchLabels();
                if (branchLabels.size() >= 1)
                    ImGui::TextWrapped("1. %s - NPC has this relationship level or better", branchLabels[0].c_str());
                if (branchLabels.size() >= 2)
                    ImGui::TextWrapped("2. %s - NPC has a worse relationship", branchLabels[1].c_str());
            }
            if (changed || conditionTypeChanged) {
                int branchCount = static_cast<int>(node->getChildren().size());
                if (branchCount < 2) branchCount = 2;
                m_dialogueManager->createConditionBranches(node, branchCount);
            }
            break;
        }
        case ConditionType::TimeOfDay: {
            const char* timeOptions[] = { "Morning", "Afternoon", "Evening", "Night" };
            static int selectedTime = 0;
            if (!condition.parameterName.empty()) {
                for (int i = 0; i < IM_ARRAYSIZE(timeOptions); i++) {
                    if (condition.parameterName == timeOptions[i]) {
                        selectedTime = i;
                        break;
                    }
                }
            }
            bool changed = false;
            ImGui::TextUnformatted("Time of Day:");
            ImGui::SameLine();
            float timeDropdownWidth = ImGui::GetContentRegionAvail().x * 0.80f;
            ImGui::PushItemWidth(timeDropdownWidth);
            if (ImGui::Combo("##TimeOfDay", &selectedTime, timeOptions, IM_ARRAYSIZE(timeOptions))) {
                condition.parameterName = timeOptions[selectedTime];
                node->setCondition(condition);
                changed = true;
            }
            ImGui::PopItemWidth();

            ImGui::TextColored(ImVec4(0.0f, 0.8f, 0.8f, 1.0f),
                "This checks if the current game time is:");
            ImGui::TextColored(ImVec4(0.0f, 0.9f, 0.0f, 1.0f), "  %s", timeOptions[selectedTime]);

            ImGui::Separator();
            ImGui::TextColored(ImVec4(0.8f, 0.7f, 0.0f, 1.0f), "Branch outcomes:");
            {
                auto branchLabels = condition.GetBranchLabels();
                if (branchLabels.size() >= 1)
                    ImGui::TextWrapped("1. %s - Current time matches", branchLabels[0].c_str());
                if (branchLabels.size() >= 2)
                    ImGui::TextWrapped("2. %s - Current time doesn't match", branchLabels[1].c_str());
            }
            if (changed || conditionTypeChanged) {
                int branchCount = static_cast<int>(node->getChildren().size());
                if (branchCount < 2) branchCount = 2;
                m_dialogueManager->createConditionBranches(node, branchCount);
            }
            break;
        }
        case ConditionType::QuestStatus: {
            std::string questId = condition.parameterName;
            std::string questStatus = condition.parameterValue;
            bool changed = false;
            ImGui::TextUnformatted("Quest ID:");
            ImGui::SameLine();
            float inputWidth = ImGui::GetContentRegionAvail().x * 0.80f;
            ImGui::PushItemWidth(inputWidth);
            if (ImGui::InputText("##QuestID", &questId)) {
                condition.parameterName = questId;
                node->setCondition(condition);
                changed = true;
            }
            ImGui::PopItemWidth();

            ImGui::TextUnformatted("Quest Status:");
            ImGui::SameLine();
            ImGui::PushItemWidth(inputWidth);
            const char* statusOptions[] = { "Not Started", "In Progress", "Completed", "Failed" };
            static int selectedStatus = 0;
            if (!questStatus.empty()) {
                for (int i = 0; i < IM_ARRAYSIZE(statusOptions); i++) {
                    if (questStatus == statusOptions[i]) {
                        selectedStatus = i;
                        break;
                    }
                }
            }
            if (ImGui::Combo("##QuestStatus", &selectedStatus, statusOptions, IM_ARRAYSIZE(statusOptions))) {
                condition.parameterValue = statusOptions[selectedStatus];
                node->setCondition(condition);
                changed = true;
            }
            ImGui::PopItemWidth();

            ImGui::TextColored(ImVec4(0.0f, 0.8f, 0.8f, 1.0f),
                "This checks if quest \"%s\" is:", questId.c_str());
            ImGui::TextColored(ImVec4(0.0f, 0.9f, 0.0f, 1.0f), "  %s", statusOptions[selectedStatus]);

            ImGui::Separator();
            ImGui::TextColored(ImVec4(0.8f, 0.7f, 0.0f, 1.0f), "Branch outcomes:");
            {
                auto branchLabels = condition.GetBranchLabels();
                if (branchLabels.size() >= 1)
                    ImGui::TextWrapped("1. %s - Quest has this status", branchLabels[0].c_str());
                if (branchLabels.size() >= 2)
                    ImGui::TextWrapped("2. %s - Quest has a different status", branchLabels[1].c_str());
            }
            if (changed || conditionTypeChanged) {
                int branchCount = static_cast<int>(node->getChildren().size());
                if (branchCount < 2) branchCount = 2;
                m_dialogueManager->createConditionBranches(node, branchCount);
            }
            break;
        }
        case ConditionType::InventoryCheck: {
            std::string itemId = condition.parameterName;
            std::string quantity = condition.parameterValue;
            bool changed = false;
            ImGui::TextUnformatted("Item ID:");
            ImGui::SameLine();
            float inputWidth = ImGui::GetContentRegionAvail().x * 0.80f;
            ImGui::PushItemWidth(inputWidth);
            if (ImGui::InputText("##ItemID", &itemId)) {
                condition.parameterName = itemId;
                node->setCondition(condition);
                changed = true;
            }
            ImGui::PopItemWidth();

            ImGui::TextUnformatted("Minimum Quantity:");
            ImGui::SameLine();
            ImGui::PushItemWidth(inputWidth * 0.67f);
            if (ImGui::InputText("##Quantity", &quantity)) {
                condition.parameterValue = quantity;
                node->setCondition(condition);
                changed = true;
            }
            ImGui::PopItemWidth();

            int quantityValue = 1;
            try {
                quantityValue = std::stoi(quantity);
            }
            catch (...) {}

            ImGui::TextColored(ImVec4(0.0f, 0.8f, 0.8f, 1.0f),
                "This checks if player has at least: %d %s", quantityValue, itemId.c_str());

            ImGui::Separator();
            ImGui::TextColored(ImVec4(0.8f, 0.7f, 0.0f, 1.0f), "Branch outcomes:");
            {
                auto branchLabels = condition.GetBranchLabels();
                if (branchLabels.size() >= 1)
                    ImGui::TextWrapped("1. %s - Player has enough items", branchLabels[0].c_str());
                if (branchLabels.size() >= 2)
                    ImGui::TextWrapped("2. %s - Player doesn't have enough", branchLabels[1].c_str());
            }
            if (changed || conditionTypeChanged) {
                int branchCount = static_cast<int>(node->getChildren().size());
                if (branchCount < 2) branchCount = 2;
                m_dialogueManager->createConditionBranches(node, branchCount);
            }
            break;
        }
        case ConditionType::StatCheck: {
            std::string statName = condition.parameterName;
            std::string threshold = condition.parameterValue;
            bool changed = false;
            ImGui::TextUnformatted("Stat Name:");
            ImGui::SameLine();
            float inputWidth = ImGui::GetContentRegionAvail().x * 0.80f;
            ImGui::PushItemWidth(inputWidth);
            if (ImGui::InputText("##StatName", &statName)) {
                condition.parameterName = statName;
                node->setCondition(condition);
                changed = true;
            }
            ImGui::PopItemWidth();
            ImGui::TextUnformatted("Minimum Value:");
            ImGui::SameLine();
            ImGui::PushItemWidth(inputWidth);
            if (ImGui::InputText("##Threshold", &threshold)) {
                condition.parameterValue = threshold;
                node->setCondition(condition);
                changed = true;
            }
            ImGui::PopItemWidth();

            int thresholdValue = 1;
            try {
                thresholdValue = std::stoi(threshold);
            }
            catch (...) {}

            ImGui::TextColored(ImVec4(0.0f, 0.8f, 0.8f, 1.0f),
                "This checks if player's stat is at least: %s >= %d", statName.c_str(), thresholdValue);

            ImGui::Separator();
            ImGui::TextColored(ImVec4(0.8f, 0.7f, 0.0f, 1.0f), "Branch outcomes:");
            {
                auto branchLabels = condition.GetBranchLabels();
                if (branchLabels.size() >= 1)
                    ImGui::TextWrapped("1. %s - Stat is high enough", branchLabels[0].c_str());
                if (branchLabels.size() >= 2)
                    ImGui::TextWrapped("2. %s - Stat is too low", branchLabels[1].c_str());
            }
            if (changed || conditionTypeChanged) {
                int branchCount = static_cast<int>(node->getChildren().size());
                if (branchCount < 2) branchCount = 2;
                m_dialogueManager->createConditionBranches(node, branchCount);
            }
            break;
        }
        case ConditionType::Custom: {
            std::string paramName = condition.parameterName;
            std::string paramValue = condition.parameterValue;
            bool changed = false;
            ImGui::TextUnformatted("Parameter Name:");
            ImGui::SameLine();
            float inputWidth = ImGui::GetContentRegionAvail().x * 0.80f;
            ImGui::PushItemWidth(inputWidth);
            if (ImGui::InputText("##ParamName", &paramName)) {
                condition.parameterName = paramName;
                node->setCondition(condition);
                changed = true;
            }
            ImGui::PopItemWidth();
            ImGui::TextUnformatted("Parameter Value:");
            ImGui::SameLine();
            ImGui::PushItemWidth(inputWidth);
            if (ImGui::InputText("##ParamValue", &paramValue)) {
                condition.parameterValue = paramValue;
                node->setCondition(condition);
                changed = true;
            }
            ImGui::PopItemWidth();
            ImGui::TextColored(ImVec4(0.0f, 0.8f, 0.8f, 1.0f), "This checks custom condition:");
            ImGui::TextColored(ImVec4(0.0f, 0.9f, 0.0f, 1.0f), "  %s = %s", paramName.c_str(), paramValue.c_str());
            ImGui::Separator();
            ImGui::TextColored(ImVec4(0.8f, 0.7f, 0.0f, 1.0f), "Branch outcomes:");
            {
                auto branchLabels = condition.GetBranchLabels();
                if (branchLabels.size() >= 1)
                    ImGui::TextWrapped("1. %s - Condition is true", branchLabels[0].c_str());
                if (branchLabels.size() >= 2)
                    ImGui::TextWrapped("2. %s - Condition is false", branchLabels[1].c_str());
            }
            if (changed || conditionTypeChanged) {
                int branchCount = static_cast<int>(node->getChildren().size());
                if (branchCount < 2) branchCount = 2;
                m_dialogueManager->createConditionBranches(node, branchCount);
            }
            break;
        }
        default:
            break;
        } // end switch condition.type


        // Branch management section.
        ImGui::Separator();
        ImGui::TextWrapped("Branch Management:");
        int branchCount = static_cast<int>(node->getChildren().size());
        if (branchCount < 1) branchCount = 2;
        ImGui::TextWrapped("Current branches: %d", branchCount);
        if (ImGui::Button("Regenerate Default Branches")) {
            auto branchLabels = condition.GetBranchLabels();
            m_dialogueManager->createConditionBranches(node, branchLabels.size());
        }
        ImGui::SameLine();
        if (ImGui::Button("Add Extra Branch")) {
            m_dialogueManager->createConditionBranches(node, branchCount + 1);
        }
        ImGui::Separator();
        ImGui::TextColored(ImVec4(0.0f, 0.8f, 0.0f, 1.0f), "Condition: %s", condition.GetDescription().c_str());
        ImGui::Separator();
        ImGui::TextColored(ImVec4(0.8f, 0.7f, 0.0f, 1.0f), "Branch-Specific Settings:");

        auto children = node->getChildren();
        for (int i = 0; i < children.size(); i++) {
            ImGui::PushID(i);

            std::string branchLabel = children[i].second;
            ImGui::TextWrapped("Branch %d: %s", i + 1, branchLabel.c_str());

            // Get branch-specific condition and initialize defaults if needed.
            BranchCondition branchCond = node->getBranchCondition(i);
            if (branchCond.type != condition.type) {
                branchCond.type = condition.type;
                switch (condition.type) {
                case ConditionType::RelationshipStatus: {
                    // Declare the levels locally.
                    const char* relationshipLevels[] = { "Hostile", "Unfriendly", "Neutral", "Friendly", "Close" };
                    if (i == 0) {
                        branchCond = BranchCondition::CreateRelationshipRange(
                            condition.relationshipThreshold, RelationshipStatus::Close
                        );
                    }
                    else if (i == 1 && condition.relationshipThreshold > RelationshipStatus::Hostile) {
                        RelationshipStatus belowThreshold = static_cast<RelationshipStatus>(
                            static_cast<int>(condition.relationshipThreshold) - 1
                            );
                        branchCond = BranchCondition::CreateRelationshipRange(belowThreshold, belowThreshold);
                    }
                    else if (i == 2 && condition.relationshipThreshold > RelationshipStatus::Unfriendly) {
                        RelationshipStatus evenLower = static_cast<RelationshipStatus>(
                            std::max(0, static_cast<int>(condition.relationshipThreshold) - 2)
                            );
                        branchCond = BranchCondition::CreateRelationshipRange(evenLower, evenLower);
                    }
                    else {
                        branchCond = BranchCondition::CreateRelationshipRange(
                            RelationshipStatus::Hostile, RelationshipStatus::Hostile
                        );
                    }
                    break;
                }
                case ConditionType::InventoryCheck: {
                    int reqQuantity = 0;
                    try {
                        reqQuantity = std::stoi(condition.parameterValue);
                    }
                    catch (...) {
                        reqQuantity = 1;
                    }
                    if (i == 0) {
                        branchCond = BranchCondition::CreateInventoryRange(condition.parameterName, reqQuantity, 9999);
                    }
                    else if (i == 1) {
                        if (reqQuantity > 1)
                            branchCond = BranchCondition::CreateInventoryRange(condition.parameterName, 1, reqQuantity - 1);
                        else
                            branchCond = BranchCondition::CreateInventoryRange(condition.parameterName, 0, 0);
                    }
                    else {
                        branchCond = BranchCondition::CreateInventoryRange(condition.parameterName, 0, 0);
                    }
                    break;
                }
                case ConditionType::StatCheck: {
                    int threshold = 0;
                    try {
                        threshold = std::stoi(condition.parameterValue);
                    }
                    catch (...) {
                        threshold = 1;
                    }
                    if (i == 0) {
                        branchCond = BranchCondition::CreateStatRange(condition.parameterName, threshold, 9999);
                    }
                    else if (i == 1 && threshold > 1) {
                        int lowerBound = std::max(1, threshold - (threshold / 2));
                        branchCond = BranchCondition::CreateStatRange(condition.parameterName, lowerBound, threshold - 1);
                    }
                    else {
                        branchCond = BranchCondition::CreateStatRange(condition.parameterName, 0, std::max(0, threshold - 1));
                    }
                    break;
                }
                case ConditionType::QuestStatus: {
                    if (i == 0) {
                        branchCond = BranchCondition::CreateQuestStatus(condition.parameterName, condition.parameterValue);
                    }
                    else {
                        std::string otherStatus = "Other";
                        const std::vector<std::string> statuses = {
                            "Not Started", "In Progress", "Completed", "Failed"
                        };
                        for (const auto& status : statuses) {
                            if (status != condition.parameterValue) {
                                otherStatus = status;
                                break;
                            }
                        }
                        branchCond = BranchCondition::CreateQuestStatus(condition.parameterName, otherStatus);
                    }
                    break;
                }
                case ConditionType::TimeOfDay: {
                    if (i == 0) {
                        branchCond = BranchCondition::CreateTimeOfDay(condition.parameterName);
                    }
                    else {
                        std::string otherTime = "Other";
                        const std::vector<std::string> times = { "Morning", "Afternoon", "Evening", "Night" };
                        for (const auto& time : times) {
                            if (time != condition.parameterName) {
                                otherTime = time;
                                break;
                            }
                        }
                        branchCond = BranchCondition::CreateTimeOfDay(otherTime);
                    }
                    break;
                }
                case ConditionType::Custom: {
                    if (i == 0)
                        branchCond = BranchCondition::CreateCustomCondition(condition.parameterName, "True");
                    else
                        branchCond = BranchCondition::CreateCustomCondition(condition.parameterName, "False");
                    break;
                }
                default:
                    break;
                }
                node->setBranchCondition(i, branchCond);
            }

            // Branch-specific UI based on condition type.
            switch (condition.type) {
            case ConditionType::RelationshipStatus: {
                // Declare relationship levels.
                const char* relationshipLevels[] = { "Hostile", "Unfriendly", "Neutral", "Friendly", "Close" };
                // Min Relationship
                ImGui::TextUnformatted("Min Relationship:");
                ImGui::SameLine();
                float relWidth = ImGui::GetContentRegionAvail().x * 0.80f;
                ImGui::PushItemWidth(relWidth);
                int minLevel = static_cast<int>(branchCond.minRelationship);
                if (ImGui::Combo("##MinRel", &minLevel, relationshipLevels, IM_ARRAYSIZE(relationshipLevels))) {
                    if (minLevel > static_cast<int>(branchCond.maxRelationship))
                        branchCond.maxRelationship = static_cast<RelationshipStatus>(minLevel);
                    branchCond.minRelationship = static_cast<RelationshipStatus>(minLevel);
                    branchCond.description = BranchCondition::GetRelationshipRangeDescription(branchCond.minRelationship, branchCond.maxRelationship);
                    node->setBranchCondition(i, branchCond);
                    node->updateChildCondition(children[i].first->getId(),
                        BranchCondition::GetRelationshipName(branchCond.minRelationship) +
                        (branchCond.minRelationship != branchCond.maxRelationship ? "+" : ""));
                }
                ImGui::PopItemWidth();

                // Max Relationship
                ImGui::TextUnformatted("Max Relationship:");
                ImGui::SameLine();
                ImGui::PushItemWidth(relWidth);
                int maxLevel = static_cast<int>(branchCond.maxRelationship);
                if (ImGui::Combo("##MaxRel", &maxLevel, relationshipLevels, IM_ARRAYSIZE(relationshipLevels))) {
                    if (maxLevel < static_cast<int>(branchCond.minRelationship))
                        branchCond.minRelationship = static_cast<RelationshipStatus>(maxLevel);
                    branchCond.maxRelationship = static_cast<RelationshipStatus>(maxLevel);
                    branchCond.description = BranchCondition::GetRelationshipRangeDescription(branchCond.minRelationship, branchCond.maxRelationship);
                    node->setBranchCondition(i, branchCond);
                    node->updateChildCondition(children[i].first->getId(),
                        BranchCondition::GetRelationshipName(branchCond.minRelationship) +
                        (branchCond.minRelationship != branchCond.maxRelationship ? "+" : ""));
                }
                ImGui::PopItemWidth();
                ImGui::TextColored(ImVec4(0.0f, 0.7f, 0.7f, 1.0f), "This branch is used when: %s", branchCond.description.c_str());
                break;
            }
            case ConditionType::InventoryCheck: {
                // (No wrapping changes beyond what was already applied.)
                ImGui::TextUnformatted("Item:");
                ImGui::SameLine();
                float inputWidth = ImGui::GetContentRegionAvail().x * 0.80f;
                ImGui::PushItemWidth(inputWidth);
                std::string itemId = branchCond.itemId.empty() ? condition.parameterName : branchCond.itemId;
                if (ImGui::InputText("##Item", &itemId)) {
                    branchCond.itemId = itemId;
                    node->setBranchCondition(i, branchCond);
                    updateInventoryBranchLabel(node, children[i].first->getId(), branchCond);
                }
                ImGui::PopItemWidth();

                static bool isItemRange[100] = { true };
                bool isRange = isItemRange[i];
                if (ImGui::Checkbox("Use Quantity Range##itemrange", &isRange)) {
                    isItemRange[i] = isRange;
                    if (!isRange) {
                        branchCond.maxQuantity = branchCond.minQuantity;
                        node->setBranchCondition(i, branchCond);
                        updateInventoryBranchLabel(node, children[i].first->getId(), branchCond);
                    }
                }
                if (isRange) {
                    ImGui::TextUnformatted("Min Quantity:");
                    ImGui::SameLine();
                    float halfWidth = ImGui::GetContentRegionAvail().x * 0.80f;
                    ImGui::PushItemWidth(halfWidth);
                    int minQty = branchCond.minQuantity;
                    if (ImGui::InputInt("##MinQty", &minQty)) {
                        if (minQty < 0) minQty = 0;
                        if (minQty > branchCond.maxQuantity) branchCond.maxQuantity = minQty;
                        branchCond.minQuantity = minQty;
                        branchCond.description = BranchCondition::GetInventoryRangeDescription(branchCond.itemId, minQty, branchCond.maxQuantity);
                        node->setBranchCondition(i, branchCond);
                        updateInventoryBranchLabel(node, children[i].first->getId(), branchCond);
                    }
                    ImGui::PopItemWidth();

                    ImGui::TextUnformatted("Max Quantity:");
                    ImGui::SameLine();
                    ImGui::PushItemWidth(halfWidth);
                    int maxQty = branchCond.maxQuantity;
                    if (ImGui::InputInt("##MaxQty", &maxQty)) {
                        if (maxQty < branchCond.minQuantity) branchCond.minQuantity = maxQty;
                        branchCond.maxQuantity = maxQty;
                        branchCond.description = BranchCondition::GetInventoryRangeDescription(branchCond.itemId, branchCond.minQuantity, maxQty);
                        node->setBranchCondition(i, branchCond);
                        updateInventoryBranchLabel(node, children[i].first->getId(), branchCond);
                    }
                    ImGui::PopItemWidth();
                }
                else {
                    ImGui::TextUnformatted("Exact Quantity:");
                    ImGui::SameLine();
                    ImGui::PushItemWidth(inputWidth * 0.67f);
                    int exactQty = branchCond.minQuantity;
                    if (ImGui::InputInt("##ExactQty", &exactQty)) {
                        if (exactQty < 0) exactQty = 0;
                        branchCond.minQuantity = exactQty;
                        branchCond.maxQuantity = exactQty;
                        branchCond.description = BranchCondition::GetInventoryRangeDescription(branchCond.itemId, exactQty, exactQty);
                        node->setBranchCondition(i, branchCond);
                        updateInventoryBranchLabel(node, children[i].first->getId(), branchCond);
                    }
                    ImGui::PopItemWidth();
                }
                ImGui::TextColored(ImVec4(0.0f, 0.7f, 0.7f, 1.0f), "This branch is used when: %s", branchCond.description.c_str());
                bool noItem = (branchCond.minQuantity == 0 && branchCond.maxQuantity == 0);
                if (ImGui::Checkbox("No item shortcut", &noItem)) {
                    if (noItem) {
                        branchCond.minQuantity = 0;
                        branchCond.maxQuantity = 0;
                    }
                    else {
                        branchCond.minQuantity = 1;
                        branchCond.maxQuantity = 9999;
                    }
                    branchCond.description = BranchCondition::GetInventoryRangeDescription(branchCond.itemId, branchCond.minQuantity, branchCond.maxQuantity);
                    node->setBranchCondition(i, branchCond);
                    std::string label = noItem ? "No" : "Has 1+";
                    node->updateChildCondition(children[i].first->getId(), label + " " + itemId);
                }
                break;
            }
            case ConditionType::StatCheck: {
                std::string statName = branchCond.statName.empty() ? condition.parameterName : branchCond.statName;
                float inputWidth = ImGui::GetContentRegionAvail().x * 0.80f;
                ImGui::TextUnformatted("Stat Name:");
                ImGui::SameLine();
                ImGui::PushItemWidth(inputWidth);
                if (ImGui::InputText("##StatName", &statName)) {
                    branchCond.statName = statName;
                    node->setBranchCondition(i, branchCond);
                    updateStatBranchLabel(node, children[i].first->getId(), branchCond);
                }
                ImGui::PopItemWidth();

                static bool isStatRange[100] = { true };
                bool isRange = isStatRange[i];
                if (ImGui::Checkbox("Use Stat Range##statrange", &isRange)) {
                    isStatRange[i] = isRange;
                    if (!isRange && branchCond.minValue != branchCond.maxValue) {
                        branchCond.maxValue = branchCond.minValue;
                        node->setBranchCondition(i, branchCond);
                        updateStatBranchLabel(node, children[i].first->getId(), branchCond);
                    }
                }
                if (isRange) {
                    ImGui::TextUnformatted("Min Value:");
                    ImGui::SameLine();
                    float halfWidth = ImGui::GetContentRegionAvail().x * 0.80f;
                    ImGui::PushItemWidth(halfWidth);
                    int minVal = branchCond.minValue;
                    if (ImGui::InputInt("##MinVal", &minVal)) {
                        if (minVal < 0) minVal = 0;
                        if (minVal > branchCond.maxValue) branchCond.maxValue = minVal;
                        branchCond.minValue = minVal;
                        branchCond.description = BranchCondition::GetStatRangeDescription(branchCond.statName, minVal, branchCond.maxValue);
                        node->setBranchCondition(i, branchCond);
                        updateStatBranchLabel(node, children[i].first->getId(), branchCond);
                    }
                    ImGui::PopItemWidth();

                    ImGui::TextUnformatted("Max Value:");
                    ImGui::SameLine();
                    ImGui::PushItemWidth(halfWidth);
                    int maxVal = branchCond.maxValue;
                    if (ImGui::InputInt("##MaxVal", &maxVal)) {
                        if (maxVal < minVal) minVal = maxVal;
                        branchCond.minValue = minVal;
                        branchCond.maxValue = maxVal;
                        branchCond.description = BranchCondition::GetStatRangeDescription(branchCond.statName, minVal, maxVal);
                        node->setBranchCondition(i, branchCond);
                        updateStatBranchLabel(node, children[i].first->getId(), branchCond);
                    }
                    ImGui::PopItemWidth();
                }
                else {
                    ImGui::TextUnformatted("Exact Value:");
                    ImGui::SameLine();
                    ImGui::PushItemWidth(inputWidth);
                    int exactVal = branchCond.minValue;
                    if (ImGui::InputInt("##ExactVal", &exactVal)) {
                        if (exactVal < 0) exactVal = 0;
                        branchCond.minValue = exactVal;
                        branchCond.maxValue = exactVal;
                        branchCond.description = BranchCondition::GetStatRangeDescription(branchCond.statName, exactVal, exactVal);
                        node->setBranchCondition(i, branchCond);
                        updateStatBranchLabel(node, children[i].first->getId(), branchCond);
                    }
                    ImGui::PopItemWidth();
                }
                ImGui::TextColored(ImVec4(0.0f, 0.7f, 0.7f, 1.0f), "This branch is used when: %s", branchCond.description.c_str());
                if (ImGui::Button("High Stat")) {
                    int threshold = 0;
                    try { threshold = std::stoi(condition.parameterValue); }
                    catch (...) { threshold = 10; }
                    branchCond.minValue = threshold;
                    branchCond.maxValue = 9999;
                    branchCond.description = BranchCondition::GetStatRangeDescription(branchCond.statName, branchCond.minValue, branchCond.maxValue);
                    node->setBranchCondition(i, branchCond);
                    node->updateChildCondition(children[i].first->getId(), statName + " ? " + std::to_string(threshold));
                }
                ImGui::SameLine();
                if (ImGui::Button("Medium Stat")) {
                    int threshold = 0;
                    try { threshold = std::stoi(condition.parameterValue); }
                    catch (...) { threshold = 10; }
                    int lower = std::max(1, threshold / 2);
                    int upper = threshold - 1;
                    if (lower <= upper) {
                        branchCond.minValue = lower;
                        branchCond.maxValue = upper;
                        branchCond.description = BranchCondition::GetStatRangeDescription(branchCond.statName, branchCond.minValue, branchCond.maxValue);
                        node->setBranchCondition(i, branchCond);
                        node->updateChildCondition(children[i].first->getId(), statName + ": " + std::to_string(lower) + "-" + std::to_string(upper));
                    }
                }
                ImGui::SameLine();
                if (ImGui::Button("Low Stat")) {
                    branchCond.minValue = 0;
                    branchCond.maxValue = std::max(0, std::stoi(condition.parameterValue) / 2 - 1);
                    branchCond.description = BranchCondition::GetStatRangeDescription(branchCond.statName, branchCond.minValue, branchCond.maxValue);
                    node->setBranchCondition(i, branchCond);
                    node->updateChildCondition(children[i].first->getId(), statName + " ? " + std::to_string(branchCond.maxValue));
                }
                break;
            }
            case ConditionType::QuestStatus: {
                ImGui::TextUnformatted("Quest ID:");
                ImGui::SameLine();
                float inputWidth = ImGui::GetContentRegionAvail().x * 0.80f;
                ImGui::PushItemWidth(inputWidth);
                std::string questId = branchCond.questId.empty() ? condition.parameterName : branchCond.questId;
                if (ImGui::InputText("##QuestID", &questId)) {
                    branchCond.questId = questId;
                    node->setBranchCondition(i, branchCond);
                    std::string updatedLabel = questId + ": " + branchCond.questStatus;
                    node->updateChildCondition(children[i].first->getId(), updatedLabel);
                }
                ImGui::PopItemWidth();

                const char* statusOptions[] = { "Not Started", "In Progress", "Completed", "Failed" };
                int selectedStatus = 0;
                std::string questStatus = branchCond.questStatus.empty() ? condition.parameterValue : branchCond.questStatus;
                for (int s = 0; s < IM_ARRAYSIZE(statusOptions); s++) {
                    if (questStatus == statusOptions[s]) {
                        selectedStatus = s;
                        break;
                    }
                }
                ImGui::TextUnformatted("Quest Status:");
                ImGui::SameLine();
                ImGui::PushItemWidth(inputWidth);
                if (ImGui::Combo("##QuestStatus", &selectedStatus, statusOptions, IM_ARRAYSIZE(statusOptions))) {
                    branchCond.questStatus = statusOptions[selectedStatus];
                    branchCond.description = "Quest '" + questId + "' is " + branchCond.questStatus;
                    node->setBranchCondition(i, branchCond);
                    std::string updatedLabel = questId + ": " + branchCond.questStatus;
                    node->updateChildCondition(children[i].first->getId(), updatedLabel);
                }
                ImGui::PopItemWidth();

                static bool isMultiStatus[100] = { false };
                bool isMulti = isMultiStatus[i];
                if (ImGui::Checkbox("Use Multiple Statuses##multistatus", &isMulti)) {
                    isMultiStatus[i] = isMulti;
                    if (isMulti && branchCond.parameterValue.empty()) {
                        branchCond.parameterValue = branchCond.questStatus;
                        node->setBranchCondition(i, branchCond);
                    }
                }
                if (isMulti) {
                    ImGui::TextWrapped("Select additional statuses:");
                    bool statusSelected[4] = { false, false, false, false };
                    std::string multiStatus = branchCond.parameterValue;
                    for (int s = 0; s < IM_ARRAYSIZE(statusOptions); s++) {
                        if (multiStatus.find(statusOptions[s]) != std::string::npos)
                            statusSelected[s] = true;
                    }
                    bool statusChanged = false;
                    for (int s = 0; s < IM_ARRAYSIZE(statusOptions); s++) {
                        if (ImGui::Checkbox(statusOptions[s], &statusSelected[s]))
                            statusChanged = true;
                    }
                    if (statusChanged) {
                        std::string newMultiStatus;
                        for (int s = 0; s < IM_ARRAYSIZE(statusOptions); s++) {
                            if (statusSelected[s]) {
                                if (!newMultiStatus.empty())
                                    newMultiStatus += ", ";
                                newMultiStatus += statusOptions[s];
                            }
                        }
                        if (newMultiStatus.empty())
                            newMultiStatus = branchCond.questStatus;
                        branchCond.parameterValue = newMultiStatus;
                        branchCond.description = "Quest '" + questId + "' is one of: " + newMultiStatus;
                        node->setBranchCondition(i, branchCond);
                        std::string updatedLabel = questId + ": " + newMultiStatus;
                        node->updateChildCondition(children[i].first->getId(), updatedLabel);
                    }
                }
                ImGui::TextColored(ImVec4(0.0f, 0.7f, 0.7f, 1.0f), "This branch is used when: %s", branchCond.description.c_str());
                break;
            }
            case ConditionType::TimeOfDay: {
                const char* timeOptions[] = { "Morning", "Afternoon", "Evening", "Night" };
                static bool isTimeRange[100] = { false };
                bool isRange = isTimeRange[i];
                if (ImGui::Checkbox("Use Time Range##timerange", &isRange)) {
                    isTimeRange[i] = isRange;
                    if (isRange && branchCond.parameterValue.empty()) {
                        std::string currentTime = branchCond.timeOfDay.empty() ? condition.parameterName : branchCond.timeOfDay;
                        branchCond.parameterValue = "Night";
                        for (int t = 0; t < IM_ARRAYSIZE(timeOptions); t++) {
                            if (currentTime == timeOptions[t]) {
                                int maxIdx = (t + 1) % IM_ARRAYSIZE(timeOptions);
                                branchCond.parameterValue = timeOptions[maxIdx];
                                break;
                            }
                        }
                        node->setBranchCondition(i, branchCond);
                    }
                }
                if (isRange) {
                    // Start Time
                    ImGui::TextUnformatted("Start Time:");
                    ImGui::SameLine();
                    float timeDropdownWidth = ImGui::GetContentRegionAvail().x * 0.80f;
                    ImGui::PushItemWidth(timeDropdownWidth);
                    int selectedMinTime = 0;
                    std::string minTimeOfDay = branchCond.timeOfDay.empty() ? condition.parameterName : branchCond.timeOfDay;
                    for (int t = 0; t < IM_ARRAYSIZE(timeOptions); t++) {
                        if (minTimeOfDay == timeOptions[t]) {
                            selectedMinTime = t;
                            break;
                        }
                    }
                    if (ImGui::Combo("##StartTime", &selectedMinTime, timeOptions, IM_ARRAYSIZE(timeOptions))) {
                        branchCond.timeOfDay = timeOptions[selectedMinTime];
                        if (selectedMinTime > selectedMinTime) {} // dummy check
                    }
                    ImGui::PopItemWidth();

                    // End Time
                    ImGui::TextUnformatted("End Time:");
                    ImGui::SameLine();
                    ImGui::PushItemWidth(timeDropdownWidth);
                    int selectedMaxTime = 0;
                    std::string maxTimeOfDay = branchCond.parameterValue.empty() ? "Night" : branchCond.parameterValue;
                    for (int t = 0; t < IM_ARRAYSIZE(timeOptions); t++) {
                        if (maxTimeOfDay == timeOptions[t]) {
                            selectedMaxTime = t;
                            break;
                        }
                    }
                    if (ImGui::Combo("##EndTime", &selectedMaxTime, timeOptions, IM_ARRAYSIZE(timeOptions))) {
                        branchCond.parameterValue = timeOptions[selectedMaxTime];
                    }
                    ImGui::PopItemWidth();

                    // Update description.
                    if (branchCond.timeOfDay == branchCond.parameterValue)
                        branchCond.description = "Time is " + branchCond.timeOfDay;
                    else
                        branchCond.description = "Time between " + branchCond.timeOfDay + " and " + branchCond.parameterValue;
                    node->setBranchCondition(i, branchCond);
                    std::string branchLabel;
                    if (branchCond.timeOfDay == branchCond.parameterValue)
                        branchLabel = branchCond.timeOfDay;
                    else
                        branchLabel = branchCond.timeOfDay + "-" + branchCond.parameterValue;
                    node->updateChildCondition(children[i].first->getId(), branchLabel);
                }
                else {
                    ImGui::TextUnformatted("Time of Day:");
                    ImGui::SameLine();
                    float timeDropdownWidth = ImGui::GetContentRegionAvail().x * 0.80f;
                    ImGui::PushItemWidth(timeDropdownWidth);
                    int selectedTime = 0;
                    std::string timeOfDay = branchCond.timeOfDay.empty() ? condition.parameterName : branchCond.timeOfDay;
                    for (int t = 0; t < IM_ARRAYSIZE(timeOptions); t++) {
                        if (timeOfDay == timeOptions[t]) {
                            selectedTime = t;
                            break;
                        }
                    }
                    if (ImGui::Combo("##Time", &selectedTime, timeOptions, IM_ARRAYSIZE(timeOptions))) {
                        branchCond.timeOfDay = timeOptions[selectedTime];
                        branchCond.parameterValue = "";
                        branchCond.description = "Time is " + branchCond.timeOfDay;
                        node->setBranchCondition(i, branchCond);
                        node->updateChildCondition(children[i].first->getId(), branchCond.timeOfDay);
                    }
                    ImGui::PopItemWidth();
                }
                ImGui::TextColored(ImVec4(0.0f, 0.7f, 0.7f, 1.0f), "This branch is used when: %s", branchCond.description.c_str());
                break;
            }
            case ConditionType::Custom: {
                ImGui::TextUnformatted("Parameter Name:");
                ImGui::SameLine();
                float inputWidth = ImGui::GetContentRegionAvail().x * 0.80f;
                ImGui::PushItemWidth(inputWidth);
                std::string paramName = branchCond.parameterName.empty() ? condition.parameterName : branchCond.parameterName;
                if (ImGui::InputText("##ParamName", &paramName)) {
                    branchCond.parameterName = paramName;
                    node->setBranchCondition(i, branchCond);
                    std::string updatedLabel = paramName + ": " + branchCond.parameterValue;
                    node->updateChildCondition(children[i].first->getId(), updatedLabel);
                }
                ImGui::PopItemWidth();

                ImGui::TextUnformatted("Parameter Value:");
                ImGui::SameLine();
                ImGui::PushItemWidth(inputWidth);
                std::string paramValue = branchCond.parameterValue.empty() ? condition.parameterValue : branchCond.parameterValue;
                if (ImGui::InputText("##ParamValue", &paramValue)) {
                    branchCond.parameterValue = paramValue;
                    branchCond.description = "Custom: " + paramName + " = " + paramValue;
                    node->setBranchCondition(i, branchCond);
                    std::string updatedLabel = paramName + ": " + paramValue;
                    node->updateChildCondition(children[i].first->getId(), updatedLabel);
                }
                ImGui::PopItemWidth();

                ImGui::TextColored(ImVec4(0.0f, 0.7f, 0.7f, 1.0f), "This branch is used when: %s", branchCond.description.c_str());
                break;
            }
            default:
                break;
            }
            ImGui::PopID();
            ImGui::Separator();
        }

    }

    // Child node connections.
    if (!node->getChildren().empty()) {
        ImGui::Separator();
        ImGui::TextWrapped("Connected Nodes:");
        int i = 0;
        for (auto& childPair : node->getChildren()) {
            ImGui::PushID(i++);
            std::string childName = childPair.first->getText();
            if (childName.empty()) {
                childName = "Node " + std::to_string(childPair.first->getId());
            }

            // Show child node type with appropriate color.
            ImVec4 childColor;
            const char* childTypeStr;
            switch (childPair.first->getType()) {
            case DialogueNode::NodeType::NPCStatement:
                childColor = ImVec4(0.0f, 0.8f, 0.0f, 1.0f);
                childTypeStr = "NPC";
                break;
            case DialogueNode::NodeType::PlayerChoice:
                childColor = ImVec4(0.8f, 0.8f, 0.0f, 1.0f);
                childTypeStr = "Player";
                break;
            case DialogueNode::NodeType::ConditionCheck:
                childColor = ImVec4(0.8f, 0.4f, 0.0f, 1.0f);
                childTypeStr = "Condition";
                break;
            case DialogueNode::NodeType::BranchPoint:
                childColor = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
                childTypeStr = "Branch";
                break;
            default:
                childColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
                childTypeStr = "Unknown";
                break;
            }

            ImGui::Text("? ");
            ImGui::SameLine();
            ImGui::TextColored(childColor, "[%s] %s", childTypeStr, childName.c_str());

            std::string condition = childPair.second;
            if (ImGui::InputText("Branch Label", &condition)) {
                node->updateChildCondition(childPair.first->getId(), condition);
            }

            if (ImGui::Button("Select")) {
                m_selectedNodeId = childPair.first->getId();
            }
            ImGui::SameLine();
            if (ImGui::Button("Disconnect")) {
                node->removeChild(childPair.first->getId());
                ImGui::PopID();
                break; // Break since the collection is modified.
            }
            ImGui::PopID();
            ImGui::Separator();
        }
    }

    ImGui::PopTextWrapPos();
}


void DialogueEditor::loadSavedDialogue() {
    // TODO: Implement dialogue loading from JSON or binary format
}

void DialogueEditor::saveDialogue() {
    // TODO: Implement dialogue saving to JSON or binary format
}

void DialogueEditor::testDialogueNavigation() {
    // Find a root node to start with
    auto nodes = m_dialogueManager->getAllNodes();
    std::shared_ptr<DialogueNode> startNode = nullptr;

    for (auto& node : nodes) {
        if (!m_dialogueManager->hasParent(node->getId())) {
            startNode = node;
            break;
        }
    }

    if (!startNode) {
        ImGui::Text("No root nodes found. Create a dialogue tree first.");
        return;
    }

    ImGui::Text("Testing dialogue tree navigation");
    ImGui::Text("Starting from node: %s", startNode->getText().c_str());

    ImGui::Separator();
    ImGui::Text("Test Parameters:");

    // Set up test parameters - now with enhanced tabs for all condition types
    static int currentTab = 0;
    const char* tabs[] = { "Relationships", "Inventory", "Stats", "Quests", "Time" };

    ImGui::BeginTabBar("TestTabs");

    // Relationship settings tab
    if (ImGui::BeginTabItem(tabs[0])) {
        static int npcId = 1;
        static int playerId = 1;
        static int relationshipLevel = static_cast<int>(RelationshipStatus::Friendly);

        ImGui::SliderInt("NPC ID", &npcId, 1, 10);
        ImGui::SliderInt("Player ID", &playerId, 1, 10);

        const char* relationshipLevels[] = {
            "Hostile", "Unfriendly", "Neutral", "Friendly", "Close"
        };

        if (ImGui::Combo("Relationship", &relationshipLevel, relationshipLevels, IM_ARRAYSIZE(relationshipLevels))) {
            // Update the test relationship
            m_dialogueManager->setRelationshipForTesting(
                npcId, playerId, static_cast<RelationshipStatus>(relationshipLevel)
            );
        }

        ImGui::EndTabItem();
    }

    // Inventory settings tab
    if (ImGui::BeginTabItem(tabs[1])) {
        static char itemName[128] = "Gold";
        static int quantity = 10;

        ImGui::InputText("Item Name", itemName, IM_ARRAYSIZE(itemName));
        ImGui::SliderInt("Quantity", &quantity, 0, 100);

        if (ImGui::Button("Set Item Quantity")) {
            m_dialogueManager->setInventoryItemForTesting(itemName, quantity);
        }

        ImGui::SameLine();

        if (ImGui::Button("Remove Item")) {
            m_dialogueManager->setInventoryItemForTesting(itemName, 0);
        }

        // Display current inventory
        ImGui::Separator();
        ImGui::Text("Test Inventory:");

        // This would need access to the internal m_testInventory map
        // For now, just display the current setting
        ImGui::Text("%s: %d", itemName, quantity);

        ImGui::EndTabItem();
    }

    // Stats settings tab
    if (ImGui::BeginTabItem(tabs[2])) {
        static char statName[128] = "Strength";
        static int statValue = 10;

        ImGui::InputText("Stat Name", statName, IM_ARRAYSIZE(statName));
        ImGui::SliderInt("Stat Value", &statValue, 0, 100);

        if (ImGui::Button("Set Stat Value")) {
            m_dialogueManager->setPlayerStatForTesting(statName, statValue);
        }

        // Display current stats
        ImGui::Separator();
        ImGui::Text("Test Stats:");

        // This would need access to the internal m_testPlayerStats map
        ImGui::Text("%s: %d", statName, statValue);

        ImGui::EndTabItem();
    }

    // Quest settings tab
    if (ImGui::BeginTabItem(tabs[3])) {
        static char questId[128] = "MainQuest";
        static int questStatus = 0;

        ImGui::InputText("Quest ID", questId, IM_ARRAYSIZE(questId));

        const char* questStatuses[] = {
            "Not Started", "In Progress", "Completed", "Failed"
        };

        if (ImGui::Combo("Quest Status", &questStatus, questStatuses, IM_ARRAYSIZE(questStatuses))) {
            m_dialogueManager->setQuestStatusForTesting(questId, questStatuses[questStatus]);
        }

        if (ImGui::Button("Set Quest Status")) {
            m_dialogueManager->setQuestStatusForTesting(questId, questStatuses[questStatus]);
        }

        // Display current quests
        ImGui::Separator();
        ImGui::Text("Test Quests:");

        // This would need access to the internal m_testQuestStates map
        ImGui::Text("%s: %s", questId, questStatuses[questStatus]);

        ImGui::EndTabItem();
    }

    // Time settings tab
    if (ImGui::BeginTabItem(tabs[4])) {
        static int timeOfDay = 0;

        const char* times[] = {
            "Morning", "Afternoon", "Evening", "Night"
        };

        if (ImGui::Combo("Time of Day", &timeOfDay, times, IM_ARRAYSIZE(times))) {
            m_dialogueManager->setTimeOfDayForTesting(times[timeOfDay]);
        }

        ImGui::EndTabItem();
    }

    ImGui::EndTabBar();

    ImGui::Separator();

    // Navigate through the dialogue tree
    static std::shared_ptr<DialogueNode> currentNode = startNode;

    if (currentNode) {
        // Show node info
        ImVec4 nodeColor;
        switch (currentNode->getType()) {
        case DialogueNode::NodeType::NPCStatement:
            nodeColor = ImVec4(0.0f, 0.8f, 0.0f, 1.0f); // Green for NPC
            break;
        case DialogueNode::NodeType::PlayerChoice:
            nodeColor = ImVec4(0.8f, 0.8f, 0.0f, 1.0f); // Yellow for player
            break;
        case DialogueNode::NodeType::ConditionCheck:
            nodeColor = ImVec4(0.8f, 0.4f, 0.0f, 1.0f); // Orange for condition
            break;
        default:
            nodeColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f); // White for default
            break;
        }

        ImGui::TextColored(nodeColor, "Current Node: %s", currentNode->getText().c_str());

        // Display node type
        const char* nodeTypeStr = "";
        switch (currentNode->getType()) {
        case DialogueNode::NodeType::NPCStatement:
            nodeTypeStr = "NPC Statement";
            break;
        case DialogueNode::NodeType::PlayerChoice:
            nodeTypeStr = "Player Choice";
            break;
        case DialogueNode::NodeType::ConditionCheck:
            nodeTypeStr = "Condition Check";
            break;
        case DialogueNode::NodeType::BranchPoint:
            nodeTypeStr = "Branch Point";
            break;
        }

        ImGui::Text("Node Type: %s", nodeTypeStr);

        // Display associated response if any
        auto response = currentNode->getResponse();
        if (response) {
            ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.0f, 1.0f), "Response: %s",
                response->getTextForPersonality(PersonalityType::Bubbly).c_str());
        }

        ImGui::Separator();

        // For condition nodes, show evaluation result with branch-specific results
        if (currentNode->getType() == DialogueNode::NodeType::ConditionCheck) {
            // Main condition description
            DialogueCondition condition = currentNode->getCondition();
            ImGui::Text("Condition: %s", condition.GetDescription().c_str());

            // Evaluate condition
            bool mainResult = m_dialogueManager->evaluateCondition(
                condition, 1, 1  // Using default NPC/Player IDs
            );

            ImGui::TextColored(
                mainResult ? ImVec4(0.0f, 1.0f, 0.0f, 1.0f) : ImVec4(1.0f, 0.0f, 0.0f, 1.0f),
                "Main condition result: %s", mainResult ? "SUCCESS" : "FAIL"
            );

            // Get children for branch-specific evaluations
            auto children = currentNode->getChildren();
            ImGui::Text("Branch evaluations:");

            for (int i = 0; i < children.size(); i++) {
                BranchCondition branchCond = currentNode->getBranchCondition(i);

                // Evaluate branch condition
                bool branchResult = m_dialogueManager->evaluateBranchCondition(branchCond, 1, 1);

                // Branch color based on result
                ImVec4 branchColor = branchResult ?
                    ImVec4(0.0f, 1.0f, 0.0f, 1.0f) : // Green for success
                    ImVec4(1.0f, 0.0f, 0.0f, 1.0f);  // Red for failure

                ImGui::TextColored(branchColor, "Branch %d (%s): %s",
                    i + 1,
                    children[i].second.c_str(),
                    branchResult ? "ACTIVE" : "INACTIVE"
                );
            }

            // Auto-advance button for condition nodes
            if (ImGui::Button("Evaluate Condition")) {
                auto nextNode = m_dialogueManager->findNextNode(currentNode, 1, 1);
                if (nextNode) {
                    currentNode = nextNode;
                }
            }
        }

        // For player choice nodes, show all options
        if (currentNode->getType() == DialogueNode::NodeType::PlayerChoice) {
            ImGui::Text("Select a choice:");
        }

        // Show options to move to children
        auto children = currentNode->getChildren();
        if (!children.empty()) {
            for (const auto& childPair : children) {
                std::string buttonText = childPair.first->getText();
                if (!childPair.second.empty()) {
                    buttonText += " [" + childPair.second + "]";
                }

                if (ImGui::Button(buttonText.c_str(), ImVec2(ImGui::GetContentRegionAvail().x, 0))) {
                    currentNode = childPair.first;
                }
            }
        }
        else {
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "End of dialogue branch");
        }

        // Reset button
        if (ImGui::Button("Reset to Start")) {
            currentNode = startNode;
        }
    }
}

void DialogueEditor::createRelationshipTestTree() {
    // Clear any existing nodes first to avoid duplicate trees
    auto nodes = m_dialogueManager->getAllNodes();
    for (auto& node : nodes) {
        m_dialogueManager->deleteNode(node->getId());
    }

    // Create responses if they don't exist
    if (!m_dialogueManager->getResponse(ResponseType::EnthusiasticAffirmative))
        m_dialogueManager->createResponse(ResponseType::EnthusiasticAffirmative, "Absolutely!");
    if (!m_dialogueManager->getResponse(ResponseType::IndifferentAffirmative))
        m_dialogueManager->createResponse(ResponseType::IndifferentAffirmative, "Yeah, sure.");
    if (!m_dialogueManager->getResponse(ResponseType::UntrustworthyResponse))
        m_dialogueManager->createResponse(ResponseType::UntrustworthyResponse, "Why should I help you?");
    if (!m_dialogueManager->getResponse(ResponseType::Farewell))
        m_dialogueManager->createResponse(ResponseType::Farewell, "Goodbye!");
    if (!m_dialogueManager->getResponse(ResponseType::MarkOnMap))
        m_dialogueManager->createResponse(ResponseType::MarkOnMap, "I'll mark that on your map.");

    // 1. Root node: Player asks for help.
    auto rootNode = m_dialogueManager->createDialogueNode(
        DialogueNode::NodeType::PlayerChoice,
        "Can you help me find John?"
    );

    // 2. Create a condition check node for NPC relationship.
    auto relationshipNode = m_dialogueManager->createDialogueNode(
        DialogueNode::NodeType::ConditionCheck,
        "Check NPC's friendliness"
    );
    // Set the condition to check if the NPC is at least Neutral.
    relationshipNode->setCondition(
        DialogueCondition::CreateRelationshipCondition(RelationshipStatus::Neutral)
    );

    // 3. Create NPC responses based on relationship.
    // Friendly branch: NPC readily helps.
    auto friendlyNPC = m_dialogueManager->createDialogueNode(
        DialogueNode::NodeType::NPCStatement,
        "Sure, follow me. John is at the tavern by the market square."
    );
    friendlyNPC->setResponse(m_dialogueManager->getResponse(ResponseType::EnthusiasticAffirmative));

    // Neutral branch: NPC is hesitant.
    auto neutralNPC = m_dialogueManager->createDialogueNode(
        DialogueNode::NodeType::NPCStatement,
        "I know where John is, but I'm not sure if I should help you."
    );
    neutralNPC->setResponse(m_dialogueManager->getResponse(ResponseType::IndifferentAffirmative));

    // Hostile branch: NPC is openly unhelpful.
    auto hostileNPC = m_dialogueManager->createDialogueNode(
        DialogueNode::NodeType::NPCStatement,
        "Why should I help you?"
    );
    hostileNPC->setResponse(m_dialogueManager->getResponse(ResponseType::UntrustworthyResponse));

    // Connect relationship node with its three branches.
    relationshipNode->addChildNode(friendlyNPC, "Friendly+");
    relationshipNode->addChildNode(neutralNPC, "Neutral");
    relationshipNode->addChildNode(hostileNPC, "Hostile");

    // Set branch conditions for the relationship node.
    BranchCondition friendlyBranch = BranchCondition::CreateRelationshipRange(
        RelationshipStatus::Friendly, RelationshipStatus::Close
    );
    relationshipNode->setBranchCondition(0, friendlyBranch);
    BranchCondition neutralBranch = BranchCondition::CreateRelationshipRange(
        RelationshipStatus::Neutral, RelationshipStatus::Neutral
    );
    relationshipNode->setBranchCondition(1, neutralBranch);
    BranchCondition hostileBranch = BranchCondition::CreateRelationshipRange(
        RelationshipStatus::Hostile, RelationshipStatus::Unfriendly
    );
    relationshipNode->setBranchCondition(2, hostileBranch);

    // 4. Friendly branch goes directly to a directions node.
    auto directionsNode = m_dialogueManager->createDialogueNode(
        DialogueNode::NodeType::NPCStatement,
        "The tavern is by the market square."
    );
    directionsNode->setResponse(m_dialogueManager->getResponse(ResponseType::MarkOnMap));
    friendlyNPC->addChildNode(directionsNode);

    // 5. Neutral branch: Directly attach two player choice options.
    auto neutralPersuade = m_dialogueManager->createDialogueNode(
        DialogueNode::NodeType::PlayerChoice,
        "Please help me. I really need your help."
    );
    auto neutralDecline = m_dialogueManager->createDialogueNode(
        DialogueNode::NodeType::PlayerChoice,
        "Never mind. I'll manage on my own."
    );
    neutralNPC->addChildNode(neutralPersuade, "Persuade");
    neutralNPC->addChildNode(neutralDecline, "Decline");

    // NPC responses for neutral branch.
    auto neutralResponseA = m_dialogueManager->createDialogueNode(
        DialogueNode::NodeType::NPCStatement,
        "Alright, then follow me."
    );
    neutralResponseA->setResponse(m_dialogueManager->getResponse(ResponseType::EnthusiasticAffirmative));
    neutralPersuade->addChildNode(neutralResponseA);

    auto neutralResponseB = m_dialogueManager->createDialogueNode(
        DialogueNode::NodeType::NPCStatement,
        "Then I won't help you."
    );
    neutralResponseB->setResponse(m_dialogueManager->getResponse(ResponseType::Farewell));
    neutralDecline->addChildNode(neutralResponseB);

    // 6. Hostile branch: Directly attach two player choice options.
    auto hostilePersuade = m_dialogueManager->createDialogueNode(
        DialogueNode::NodeType::PlayerChoice,
        "I'm your friend—trust me!"
    );
    auto hostileRebuff = m_dialogueManager->createDialogueNode(
        DialogueNode::NodeType::PlayerChoice,
        "Get lost!"
    );
    hostileNPC->addChildNode(hostilePersuade, "Persuade");
    hostileNPC->addChildNode(hostileRebuff, "Rebuff");

    // NPC responses for hostile branch.
    auto hostileResponseA = m_dialogueManager->createDialogueNode(
        DialogueNode::NodeType::NPCStatement,
        "Fine, I'll help you out."
    );
    hostileResponseA->setResponse(m_dialogueManager->getResponse(ResponseType::EnthusiasticAffirmative));
    hostilePersuade->addChildNode(hostileResponseA);

    auto hostileResponseB = m_dialogueManager->createDialogueNode(
        DialogueNode::NodeType::NPCStatement,
        "Good, I won't waste my time."
    );
    hostileResponseB->setResponse(m_dialogueManager->getResponse(ResponseType::Farewell));
    hostileRebuff->addChildNode(hostileResponseB);

    // 7. Connect the root node to the relationship check.
    rootNode->addChildNode(relationshipNode);

    // Optionally, set up testing relationships.
    m_dialogueManager->setRelationshipForTesting(1, 1, RelationshipStatus::Friendly);   // NPC 1 is friendly
    m_dialogueManager->setRelationshipForTesting(2, 1, RelationshipStatus::Hostile);     // NPC 2 is hostile

    // Select the root node
    m_selectedNodeId = rootNode->getId();
}