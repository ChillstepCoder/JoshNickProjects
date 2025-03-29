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
        // Add node button
        if (ImGui::Button("Add Root Node")) {
            m_dialogueManager->createDialogueNode(DialogueNode::NodeType::Statement, "New root node");
        }

        ImGui::Separator();

        // Define the recursive function
        std::function<void(std::shared_ptr<DialogueNode>, int)> displayNode;

        // Fix: Use std::ref to properly reference the displayNode inside itself
        displayNode = [this, &displayNode](std::shared_ptr<DialogueNode> node, int depth) -> void {
            // Static flags inside lambda
            static ImGuiTreeNodeFlags base_flags = ImGuiTreeNodeFlags_OpenOnArrow |
                ImGuiTreeNodeFlags_OpenOnDoubleClick |
                ImGuiTreeNodeFlags_SpanAvailWidth;

            if (!node) return;

            ImGuiTreeNodeFlags flags = base_flags;

            // Get children for this node
            auto children = node->getChildren();
            if (children.empty()) {
                flags |= ImGuiTreeNodeFlags_Leaf;
            }

            // Node text + type display
            std::string nodeLabel = node->getText();
            if (nodeLabel.empty()) {
                nodeLabel = "Node " + std::to_string(node->getId());
            }

            // Display node type
            switch (node->getType()) {
            case DialogueNode::NodeType::Statement:
                nodeLabel += " [Statement]";
                break;
            case DialogueNode::NodeType::PlayerChoice:
                nodeLabel += " [Choice]";
                break;
            case DialogueNode::NodeType::DynamicStatement:
                nodeLabel += " [Dynamic]";
                break;
            }

            bool nodeOpen = ImGui::TreeNodeEx((void*)(intptr_t)node->getId(), flags, "%s", nodeLabel.c_str());

            // Node context menu
            if (ImGui::BeginPopupContextItem()) {
                if (ImGui::MenuItem("Add Child Node")) {
                    auto childNode = m_dialogueManager->createDialogueNode(
                        DialogueNode::NodeType::Statement, "New child node");
                    node->addChildNode(childNode);
                }

                if (ImGui::MenuItem("Add Choice Node")) {
                    auto childNode = m_dialogueManager->createDialogueNode(
                        DialogueNode::NodeType::PlayerChoice, "New choice");
                    node->addChildNode(childNode);
                }

                if (ImGui::MenuItem("Set Response...")) {
                    // TODO: Show response selection window
                }

                if (ImGui::MenuItem("Edit Text")) {
                    // TODO: Show text editor popup
                }

                if (depth > 0 && ImGui::MenuItem("Delete Node")) {
                    // TODO: Implement node deletion
                }

                ImGui::EndPopup();
            }

            // Display children if node is open
            if (nodeOpen) {
                for (const auto& childPair : children) {
                    // Fix: Use displayNode directly, no need for self reference
                    displayNode(childPair.first, depth + 1);
                }
                ImGui::TreePop();
            }
            };

        // Display all root nodes
        for (const auto& node : m_dialogueManager->getAllResponses()) {
            if (node) {
                //displayNode(node, 0);
            }
        }
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

void DialogueEditor::loadSavedDialogue() {
    // TODO: Implement dialogue loading from JSON or binary format
}

void DialogueEditor::saveDialogue() {
    // TODO: Implement dialogue saving to JSON or binary format
}