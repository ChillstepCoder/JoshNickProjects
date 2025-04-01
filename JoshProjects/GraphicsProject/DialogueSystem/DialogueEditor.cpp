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
            m_selectedNodeId = newNode->getId();
        }

        ImGui::SameLine();

        if (ImGui::Button("Add Player Choice")) {
            auto newNode = m_dialogueManager->createDialogueNode(
                DialogueNode::NodeType::PlayerChoice,
                "New Player Choice"
            );
            m_selectedNodeId = newNode->getId();
        }

        ImGui::SameLine();

        if (ImGui::Button("Add Condition Check")) {
            auto newNode = m_dialogueManager->createDialogueNode(
                DialogueNode::NodeType::ConditionCheck,
                "New Condition Check"
            );
            m_selectedNodeId = newNode->getId();
        }

        ImGui::SameLine();

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

void DialogueEditor::renderNodeTree() {
    // Get all root nodes (nodes without parents)
    auto allNodes = m_dialogueManager->getAllNodes();
    std::vector<std::shared_ptr<DialogueNode>> rootNodes;

    // Identify root nodes
    for (const auto& node : allNodes) {
        if (!m_dialogueManager->hasParent(node->getId())) {
            rootNodes.push_back(node);
        }
    }

    // If no root nodes, show a message
    if (rootNodes.empty()) {
        ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "No dialogue nodes created yet. Use the buttons above to start.");
        return;
    }

    // Define the recursive rendering function
    std::function<void(std::shared_ptr<DialogueNode>, int)> renderNodeRecursive;

    renderNodeRecursive = [this, &renderNodeRecursive](std::shared_ptr<DialogueNode> node, int depth) {
        if (!node) return;

        // Set up tree node flags
        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;

        // If this is the currently selected node, highlight it
        if (node->getId() == m_selectedNodeId) {
            flags |= ImGuiTreeNodeFlags_Selected;
        }

        // If no children, make it a leaf node
        auto children = node->getChildren();
        if (children.empty()) {
            flags |= ImGuiTreeNodeFlags_Leaf;
        }

        // Generate display text for the node
        std::string nodeText = node->getText();
        if (nodeText.empty()) {
            nodeText = "Node " + std::to_string(node->getId());
        }

        // Add node type and color information
        ImVec4 nodeColor;
        std::string typeIndicator;

        switch (node->getType()) {
        case DialogueNode::NodeType::NPCStatement:
            nodeColor = ImVec4(0.0f, 0.8f, 0.0f, 1.0f); // Green for NPC dialogue
            typeIndicator = "[NPC]";
            break;
        case DialogueNode::NodeType::PlayerChoice:
            nodeColor = ImVec4(0.8f, 0.8f, 0.0f, 1.0f); // Yellow for player choices
            typeIndicator = "[Player]";
            break;
        case DialogueNode::NodeType::ConditionCheck:
            nodeColor = ImVec4(0.8f, 0.4f, 0.0f, 1.0f); // Orange for conditions
            typeIndicator = "[Condition]";
            break;
        case DialogueNode::NodeType::BranchPoint:
            nodeColor = ImVec4(0.5f, 0.5f, 0.5f, 1.0f); // Gray for branch points
            typeIndicator = "[Branch]";
            break;
        }

        // Show condition information for condition nodes
        if (node->getType() == DialogueNode::NodeType::ConditionCheck) {
            nodeText += " (" + node->getCondition().GetDescription() + ")";
        }

        // For NPC statement nodes that have responses, indicate which response is used
        if (node->getType() == DialogueNode::NodeType::NPCStatement) {
            auto response = node->getResponse();
            if (response) {
                nodeText += " ? " + m_dialogueManager->getResponseTypeName(response->getType());
            }
        }

        // Display the tree node with colored text
        ImGui::PushStyleColor(ImGuiCol_Text, nodeColor);
        bool nodeOpen = ImGui::TreeNodeEx((void*)(intptr_t)node->getId(), flags, "%s %s",
            nodeText.c_str(), typeIndicator.c_str());
        ImGui::PopStyleColor();

        // Handle node selection
        if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
            m_selectedNodeId = node->getId();
        }

        // Show context menu for node operations
        if (ImGui::BeginPopupContextItem()) {
            // Add various child node types
            if (ImGui::BeginMenu("Add Child Node")) {
                if (ImGui::MenuItem("NPC Statement")) {
                    auto childNode = m_dialogueManager->createDialogueNode(
                        DialogueNode::NodeType::NPCStatement,
                        "New NPC Statement"
                    );
                    node->addChildNode(childNode);
                }

                if (ImGui::MenuItem("Player Choice")) {
                    auto childNode = m_dialogueManager->createDialogueNode(
                        DialogueNode::NodeType::PlayerChoice,
                        "New Player Choice"
                    );
                    node->addChildNode(childNode);
                }

                if (ImGui::MenuItem("Condition Check")) {
                    auto childNode = m_dialogueManager->createDialogueNode(
                        DialogueNode::NodeType::ConditionCheck,
                        "New Condition Check"
                    );
                    node->addChildNode(childNode);
                }

                if (ImGui::MenuItem("Branch Point")) {
                    auto childNode = m_dialogueManager->createDialogueNode(
                        DialogueNode::NodeType::BranchPoint,
                        "New Branch Point"
                    );
                    node->addChildNode(childNode);
                }

                ImGui::EndMenu();
            }

            // Duplicate node
            if (ImGui::MenuItem("Duplicate Node")) {
                auto newNode = m_dialogueManager->duplicateNode(node->getId());
                if (newNode) {
                    // If this is a root node, it's already added to the tree
                    // Otherwise, add it as a sibling to the current node
                    if (m_dialogueManager->hasParent(node->getId())) {
                        auto parent = m_dialogueManager->findParentNode(node->getId());
                        if (parent) {
                            parent->addChildNode(newNode, "Duplicated");
                        }
                    }
                    m_selectedNodeId = newNode->getId();
                }
            }

            ImGui::Separator();

            // Delete node
            if (ImGui::MenuItem("Delete Node")) {
                m_dialogueManager->deleteNode(node->getId());
                m_selectedNodeId = -1;
                ImGui::EndPopup();
                if (nodeOpen) {
                    ImGui::TreePop();
                }
                return;
            }

            ImGui::EndPopup();
        }

        // Recursively render child nodes if the node is open
        if (nodeOpen) {
            for (auto& childPair : children) {
                // If there's a condition label, display it before the child
                if (!childPair.second.empty()) {
                    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "?? [%s] ??", childPair.second.c_str());
                }
                renderNodeRecursive(childPair.first, depth + 1);
            }
            ImGui::TreePop();
        }
        };

    // Render all root nodes
    for (auto& rootNode : rootNodes) {
        renderNodeRecursive(rootNode, 0);
    }
}

void DialogueEditor::renderNodeProperties() {
    // Only show properties if a node is selected
    if (m_selectedNodeId < 0) {
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Select a node to edit its properties");
        return;
    }

    auto node = m_dialogueManager->getNodeById(m_selectedNodeId);
    if (!node) {
        m_selectedNodeId = -1; // Reset selection if node doesn't exist
        return;
    }

    ImGui::Text("Node Properties (ID: %d)", m_selectedNodeId);

    // Node type selector
    const char* nodeTypes[] = {
        "NPC Statement",
        "Player Choice",
        "Condition Check",
        "Branch Point"
    };
    int currentType = static_cast<int>(node->getType());
    if (ImGui::Combo("Node Type", &currentType, nodeTypes, IM_ARRAYSIZE(nodeTypes))) {
        // Update node type
        m_dialogueManager->updateNodeType(m_selectedNodeId, static_cast<DialogueNode::NodeType>(currentType));
    }

    // Node text editor
    std::string nodeText = node->getText();
    ImGui::Text("Node Text:");
    if (node->getType() == DialogueNode::NodeType::PlayerChoice) {
        ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.0f, 1.0f), "This is what the player will select");
    }
    else if (node->getType() == DialogueNode::NodeType::NPCStatement) {
        ImGui::TextColored(ImVec4(0.0f, 0.8f, 0.0f, 1.0f), "This is what the NPC will say");
    }

    if (ImGui::InputTextMultiline("##NodeText", &nodeText, ImVec2(-1.0f, 60.0f))) {
        node->setText(nodeText);
    }

    // Response selector (only for NPC statements)
    if (node->getType() == DialogueNode::NodeType::NPCStatement) {
        ImGui::Separator();
        ImGui::Text("Associated Response Type:");

        // Get all response types for selection
        std::vector<ResponseType> responseTypes;
        for (int i = 0; i <= static_cast<int>(ResponseType::AskToFollow); i++) {
            responseTypes.push_back(static_cast<ResponseType>(i));
        }

        // Find current response
        auto currentResponse = node->getResponse();
        ResponseType selectedType = currentResponse ? currentResponse->getType() : ResponseType::EnthusiasticAffirmative;

        // Create a simple combo box for response type selection
        std::string selectedTypeName = m_dialogueManager->getResponseTypeName(selectedType);
        if (ImGui::BeginCombo("Response Type", selectedTypeName.c_str())) {
            for (ResponseType type : responseTypes) {
                bool isSelected = (selectedType == type);
                std::string typeName = m_dialogueManager->getResponseTypeName(type);
                if (ImGui::Selectable(typeName.c_str(), isSelected)) {
                    // Set the selected response
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

        // Display personality settings for response
        ImGui::Text("Personality & Voice Options:");

        // Personality selector
        if (currentResponse) {
            // Helper UI for testing different personalities
            static int currentPersonality = static_cast<int>(PersonalityType::Bubbly);
            const char* personalities[] = {
                "Bubbly", "Grumpy", "Manic", "Shy", "Serious",
                "Anxious", "Confident", "Intellectual", "Mysterious", "Friendly"
            };

            if (ImGui::Combo("Test Personality", &currentPersonality, personalities, IM_ARRAYSIZE(personalities))) {
                // Just update the UI, doesn't affect the actual dialogue flow
            }

            // Show the response text for this personality
            PersonalityType personality = static_cast<PersonalityType>(currentPersonality);
            std::string responseText = currentResponse->getTextForPersonality(personality);

            ImGui::TextColored(ImVec4(0.0f, 0.8f, 0.0f, 1.0f),
                "Response text: \"%s\"", responseText.c_str());
        }
    }

    // Condition editor (only for condition nodes)
    if (node->getType() == DialogueNode::NodeType::ConditionCheck) {
        ImGui::Separator();
        ImGui::Text("Condition Properties:");

        DialogueCondition condition = node->getCondition();

        // Condition type selector
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
        if (ImGui::Combo("Condition Type", &currentCondType, conditionTypes, IM_ARRAYSIZE(conditionTypes))) {
            condition.type = static_cast<ConditionType>(currentCondType);
            node->setCondition(condition);
        }

        // Condition-specific parameters
        switch (condition.type) {
        case ConditionType::RelationshipStatus: {
            const char* relationshipLevels[] = {
                "Hostile",
                "Unfriendly",
                "Neutral",
                "Friendly",
                "Close"
            };

            int currentLevel = static_cast<int>(condition.relationshipThreshold);
            if (ImGui::Combo("Minimum Relationship", &currentLevel, relationshipLevels, IM_ARRAYSIZE(relationshipLevels))) {
                condition.relationshipThreshold = static_cast<RelationshipStatus>(currentLevel);
                node->setCondition(condition);
            }
            break;
        }

        case ConditionType::QuestStatus:
        case ConditionType::InventoryCheck:
        case ConditionType::StatCheck:
        case ConditionType::Custom: {
            std::string paramName = condition.parameterName;
            std::string paramValue = condition.parameterValue;

            if (ImGui::InputText("Parameter Name", &paramName)) {
                condition.parameterName = paramName;
                node->setCondition(condition);
            }

            if (ImGui::InputText("Parameter Value", &paramValue)) {
                condition.parameterValue = paramValue;
                node->setCondition(condition);
            }
            break;
        }
        }

        // Display current condition description
        ImGui::TextColored(ImVec4(0.0f, 0.8f, 0.0f, 1.0f),
            "Condition: %s", condition.GetDescription().c_str());
    }

    // Child node connections
    if (!node->getChildren().empty()) {
        ImGui::Separator();
        ImGui::Text("Connected Nodes:");

        int i = 0;
        for (auto& childPair : node->getChildren()) {
            ImGui::PushID(i++);

            std::string childName = childPair.first->getText();
            if (childName.empty()) {
                childName = "Node " + std::to_string(childPair.first->getId());
            }

            // Show child node type with appropriate color
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

            // Connection condition
            std::string condition = childPair.second;
            if (ImGui::InputText("Branch Label", &condition)) {
                // Update condition
                node->updateChildCondition(childPair.first->getId(), condition);
            }

            // Button to jump to this child node
            if (ImGui::Button("Select")) {
                m_selectedNodeId = childPair.first->getId();
            }

            ImGui::SameLine();

            // Button to remove this connection
            if (ImGui::Button("Disconnect")) {
                node->removeChild(childPair.first->getId());
                ImGui::PopID();
                break; // Break since we're modifying the collection
            }

            ImGui::PopID();
            ImGui::Separator();
        }
    }
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

    // Set up test parameters
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

    ImGui::Separator();

    // Navigate through the dialogue tree
    static std::shared_ptr<DialogueNode> currentNode = startNode;

    if (currentNode) {
        // Display current node
        ImGui::TextColored(ImVec4(0.0f, 0.8f, 0.0f, 1.0f), "Current Node: %s", currentNode->getText().c_str());

        // Display associated response if any
        auto response = currentNode->getResponse();
        if (response) {
            ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.0f, 1.0f), "Response: %s",
                response->getTextForPersonality(PersonalityType::Bubbly).c_str());
        }

        ImGui::Separator();

        // For condition nodes, show evaluation result
        if (currentNode->getType() == DialogueNode::NodeType::ConditionCheck) {
            bool result = m_dialogueManager->evaluateCondition(
                currentNode->getCondition(), npcId, playerId
            );

            ImGui::TextColored(
                result ? ImVec4(0.0f, 1.0f, 0.0f, 1.0f) : ImVec4(1.0f, 0.0f, 0.0f, 1.0f),
                "Condition result: %s", result ? "SUCCESS" : "FAIL"
            );

            // Auto-advance for condition nodes
            if (ImGui::Button("Evaluate Condition")) {
                auto nextNode = m_dialogueManager->findNextNode(currentNode, npcId, playerId);
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
    // Create responses if they don't exist
    if (!m_dialogueManager->getResponse(ResponseType::EnthusiasticAffirmative)) {
        m_dialogueManager->createResponse(ResponseType::EnthusiasticAffirmative, "Absolutely!");
    }

    if (!m_dialogueManager->getResponse(ResponseType::IndifferentAffirmative)) {
        m_dialogueManager->createResponse(ResponseType::IndifferentAffirmative, "Yeah, sure.");
    }

    if (!m_dialogueManager->getResponse(ResponseType::MarkOnMap)) {
        m_dialogueManager->createResponse(ResponseType::MarkOnMap, "Let me mark that on your map.");
    }

    if (!m_dialogueManager->getResponse(ResponseType::Farewell)) {
        m_dialogueManager->createResponse(ResponseType::Farewell, "See you around!");
    }

    // 1. Create initial player choice (question)
    auto rootNode = m_dialogueManager->createDialogueNode(
        DialogueNode::NodeType::PlayerChoice,
        "Do you know where John is?"
    );

    // 2. Create a condition check for NPC relationship
    auto relationshipNode = m_dialogueManager->createDialogueNode(
        DialogueNode::NodeType::ConditionCheck,
        "Check relationship with NPC"
    );

    // Set the relationship condition to check if NPC is friendly or better
    relationshipNode->setCondition(
        DialogueCondition::CreateRelationshipCondition(RelationshipStatus::Friendly)
    );

    // 3. Create NPC responses based on relationship
    // Friendly response (if relationship is good)
    auto friendlyResponse = m_dialogueManager->createDialogueNode(
        DialogueNode::NodeType::NPCStatement,
        "Of course I know John! We're good friends."
    );
    friendlyResponse->setResponse(m_dialogueManager->getResponse(ResponseType::EnthusiasticAffirmative));

    // Neutral/unfriendly response
    auto neutralResponse = m_dialogueManager->createDialogueNode(
        DialogueNode::NodeType::NPCStatement,
        "Yeah, I know where he is."
    );
    neutralResponse->setResponse(m_dialogueManager->getResponse(ResponseType::IndifferentAffirmative));

    // 4. Create the map marking response (both paths lead here)
    auto mapNode = m_dialogueManager->createDialogueNode(
        DialogueNode::NodeType::NPCStatement,
        "He's at the tavern by the market square."
    );
    mapNode->setResponse(m_dialogueManager->getResponse(ResponseType::MarkOnMap));

    // 5. NPC asks follow-up question
    auto npcQuestion = m_dialogueManager->createDialogueNode(
        DialogueNode::NodeType::NPCStatement,
        "Can I help you with anything else?"
    );

    // 6. Player choices for follow-up
    auto yesChoice = m_dialogueManager->createDialogueNode(
        DialogueNode::NodeType::PlayerChoice,
        "Yes, I have more questions"
    );

    auto noChoice = m_dialogueManager->createDialogueNode(
        DialogueNode::NodeType::PlayerChoice,
        "No, that's all I needed"
    );

    // 7. Final NPC responses
    auto moreDialogue = m_dialogueManager->createDialogueNode(
        DialogueNode::NodeType::NPCStatement,
        "What else would you like to know?"
    );

    auto endDialogue = m_dialogueManager->createDialogueNode(
        DialogueNode::NodeType::NPCStatement,
        "Good luck finding John then."
    );
    endDialogue->setResponse(m_dialogueManager->getResponse(ResponseType::Farewell));

    // Connect all the nodes together

    // Player question leads to relationship check
    rootNode->addChildNode(relationshipNode);

    // Relationship check branches based on relationship status
    relationshipNode->addChildNode(friendlyResponse, "Friendly");
    relationshipNode->addChildNode(neutralResponse, "Neutral/Unfriendly");

    // Both NPC responses lead to map marker statement
    friendlyResponse->addChildNode(mapNode);
    neutralResponse->addChildNode(mapNode);

    // After map marker, NPC asks follow-up question
    mapNode->addChildNode(npcQuestion);

    // NPC question offers player choices
    npcQuestion->addChildNode(yesChoice);
    npcQuestion->addChildNode(noChoice);

    // Player choices lead to different NPC responses
    yesChoice->addChildNode(moreDialogue);
    noChoice->addChildNode(endDialogue);

    // For testing, set up a relationship
    m_dialogueManager->setRelationshipForTesting(1, 1, RelationshipStatus::Friendly); // NPC 1 likes player 1
    m_dialogueManager->setRelationshipForTesting(2, 1, RelationshipStatus::Unfriendly); // NPC 2 dislikes player 1

    // Select the root node
    m_selectedNodeId = rootNode->getId();
}