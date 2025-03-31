//DialogueSystem.cpp

#include "DialogueSystem.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>

// Initialize static member
int DialogueNode::s_nextId = 0;

// ==========================================================
// DialogueResponse Implementation
// ==========================================================

DialogueResponse::DialogueResponse(ResponseType type, const std::string& defaultText)
    : m_type(type), m_defaultText(defaultText) {
    // Generate a name from the type
    m_name = "Response_" + std::to_string(static_cast<int>(type));
}

std::string DialogueResponse::getTextForPersonality(PersonalityType personality) const {
    auto it = m_personalityVariants.find(personality);
    if (it != m_personalityVariants.end()) {
        return it->second;
    }
    return m_defaultText;
}

void DialogueResponse::setTextForPersonality(PersonalityType personality, const std::string& text, bool isEdited) {
    m_personalityVariants[personality] = text;
    m_isEdited[personality] = isEdited;
}

bool DialogueResponse::isTextEdited(PersonalityType personality) const {
    auto it = m_isEdited.find(personality);
    if (it != m_isEdited.end()) {
        return it->second;
    }
    return false;
}

bool DialogueResponse::hasVoiceGenerated(PersonalityType personality, VoiceType voice) const {
    std::pair<PersonalityType, VoiceType> key(personality, voice);
    auto it = m_voiceFilePaths.find(key);
    if (it != m_voiceFilePaths.end()) {
        return !it->second.empty();
    }
    return false;
}

std::string DialogueResponse::getVoiceFilePath(PersonalityType personality, VoiceType voice) const {
    std::pair<PersonalityType, VoiceType> key(personality, voice);
    auto it = m_voiceFilePaths.find(key);
    if (it != m_voiceFilePaths.end()) {
        return it->second;
    }
    return "";
}

void DialogueResponse::setVoiceFilePath(PersonalityType personality, VoiceType voice, const std::string& path) {
    std::pair<PersonalityType, VoiceType> key(personality, voice);
    m_voiceFilePaths[key] = path;
}

void DialogueResponse::generateAllPersonalityVariants() {
    // Create mock personality variants
    setTextForPersonality(PersonalityType::Bubbly, m_defaultText + " [Bubbly version]", false);
    setTextForPersonality(PersonalityType::Grumpy, m_defaultText + " [Grumpy version]", false);
    setTextForPersonality(PersonalityType::Manic, m_defaultText + " [Manic version]", false);
    setTextForPersonality(PersonalityType::Shy, m_defaultText + " [Shy version]", false);
    setTextForPersonality(PersonalityType::Serious, m_defaultText + " [Serious version]", false);
    setTextForPersonality(PersonalityType::Anxious, m_defaultText + " [Anxious version]", false);
    setTextForPersonality(PersonalityType::Confident, m_defaultText + " [Confident version]", false);
    setTextForPersonality(PersonalityType::Intellectual, m_defaultText + " [Intellectual version]", false);
    setTextForPersonality(PersonalityType::Mysterious, m_defaultText + " [Mysterious version]", false);
    setTextForPersonality(PersonalityType::Friendly, m_defaultText + " [Friendly version]", false);
}

void DialogueResponse::regeneratePersonalityVariant(PersonalityType personality) {
    std::string personalityStr;

    switch (personality) {
    case PersonalityType::Bubbly: personalityStr = "Bubbly"; break;
    case PersonalityType::Grumpy: personalityStr = "Grumpy"; break;
    case PersonalityType::Manic: personalityStr = "Manic"; break;
    case PersonalityType::Shy: personalityStr = "Shy"; break;
    case PersonalityType::Serious: personalityStr = "Serious"; break;
    case PersonalityType::Anxious: personalityStr = "Anxious"; break;
    case PersonalityType::Confident: personalityStr = "Confident"; break;
    case PersonalityType::Intellectual: personalityStr = "Intellectual"; break;
    case PersonalityType::Mysterious: personalityStr = "Mysterious"; break;
    case PersonalityType::Friendly: personalityStr = "Friendly"; break;
    default: personalityStr = "Unknown"; break;
    }

    setTextForPersonality(personality, m_defaultText + " [Regenerated " + personalityStr + " version]", false);
}

// ==========================================================
// DialogueNode Implementation
// ==========================================================

DialogueNode::DialogueNode(NodeType type, const std::string& text)
    : m_type(type), m_text(text) {
    m_id = s_nextId++;
}

void DialogueNode::addChildNode(std::shared_ptr<DialogueNode> node, const std::string& condition) {
    m_children.push_back(std::make_pair(node, condition));
}

std::vector<std::pair<std::shared_ptr<DialogueNode>, std::string>> DialogueNode::getChildren() const {
    return m_children;
}

void DialogueNode::setResponse(std::shared_ptr<DialogueResponse> response) {
    m_response = response;
}

std::shared_ptr<DialogueResponse> DialogueNode::getResponse() const {
    return m_response;
}

// ==========================================================
// DialogueManager Implementation
// ==========================================================

DialogueManager::DialogueManager() : m_audioEngine(nullptr) {
}

DialogueManager::~DialogueManager() {
    shutdown();
}

bool DialogueManager::initialize(JAGEngine::WWiseAudioEngine* audioEngine) {
    m_audioEngine = audioEngine;
    return true;
}

void DialogueManager::shutdown() {
    // Clear all responses and nodes
    m_responses.clear();
    m_nodes.clear();
}

std::shared_ptr<DialogueResponse> DialogueManager::createResponse(ResponseType type, const std::string& defaultText) {
    auto response = std::make_shared<DialogueResponse>(type, defaultText);
    m_responses[type] = response;
    return response;
}

std::shared_ptr<DialogueResponse> DialogueManager::getResponse(ResponseType type) {
    auto it = m_responses.find(type);
    if (it != m_responses.end()) {
        return it->second;
    }
    return nullptr;
}

std::vector<std::shared_ptr<DialogueResponse>> DialogueManager::getAllResponses() const {
    std::vector<std::shared_ptr<DialogueResponse>> responses;
    for (const auto& pair : m_responses) {
        responses.push_back(pair.second);
    }
    return responses;
}

std::shared_ptr<DialogueNode> DialogueManager::createDialogueNode(DialogueNode::NodeType type, const std::string& text) {
    auto node = std::make_shared<DialogueNode>(type, text);
    m_nodes[node->getId()] = node;
    return node;
}

std::shared_ptr<DialogueNode> DialogueManager::getNodeById(int id) {
    auto it = m_nodes.find(id);
    if (it != m_nodes.end()) {
        return it->second;
    }
    return nullptr;
}

void DialogueManager::setAPIKeys(const std::string& gptKey, const std::string& elevenLabsKey) {
    m_gptApiKey = gptKey;
    m_elevenLabsApiKey = elevenLabsKey;
}

// Mock implementation without actual API calls
std::string DialogueManager::generateTextWithGPT(ResponseType type, PersonalityType personality, const std::string& defaultText) {
    if (m_gptApiKey.empty()) {
        std::cerr << "ERROR: No GPT API key set" << std::endl;
        return defaultText + " [API Key missing]";
    }

    std::string prompt = getPromptForPersonality(type, personality, defaultText);

    // Instead of making API call, generate mock text
    std::string personalityStr;
    switch (personality) {
    case PersonalityType::Bubbly: personalityStr = "Absolutely! For sure!"; break;
    case PersonalityType::Grumpy: personalityStr = "Yeah, whatever."; break;
    case PersonalityType::Manic: personalityStr = "YES! YES! YES!"; break;
    case PersonalityType::Shy: personalityStr = "Um, I guess so..."; break;
    case PersonalityType::Serious: personalityStr = "Affirmative."; break;
    case PersonalityType::Anxious: personalityStr = "Oh! Yes, if that's okay?"; break;
    case PersonalityType::Confident: personalityStr = "Without a doubt."; break;
    case PersonalityType::Intellectual: personalityStr = "Indeed, I concur with that assessment."; break;
    case PersonalityType::Mysterious: personalityStr = "Perhaps... if fate allows it."; break;
    case PersonalityType::Friendly: personalityStr = "Sure thing, friend!"; break;
    default: personalityStr = defaultText; break;
    }

    return personalityStr;
}

bool DialogueManager::generateVoiceWithElevenLabs(const std::string& text, PersonalityType personality, VoiceType voice, const std::string& outputPath) {
    if (m_elevenLabsApiKey.empty()) {
        std::cerr << "ERROR: No ElevenLabs API key set" << std::endl;
        return false;
    }

    std::string voiceId = getVoiceIdFromType(voice);
    if (voiceId.empty()) {
        std::cerr << "ERROR: Invalid voice type" << std::endl;
        return false;
    }

    // Mock successful voice generation by creating an empty file
    std::ofstream outFile(outputPath);
    if (!outFile) {
        std::cerr << "ERROR: Failed to open output file" << std::endl;
        return false;
    }

    outFile << "Mock audio data for: " << text << std::endl;
    outFile.close();

    return true;
}

void DialogueManager::generateAllTextVariants() {
    for (auto& pair : m_responses) {
        auto& response = pair.second;
        response->generateAllPersonalityVariants();
    }
}

void DialogueManager::generateAllVoiceVariants(VoiceType voice) {
    for (auto& pair : m_responses) {
        auto& response = pair.second;

        // For each personality type
        for (int i = 0; i < static_cast<int>(PersonalityType::Friendly) + 1; ++i) {
            PersonalityType personality = static_cast<PersonalityType>(i);

            // Get the text for this personality
            std::string text = response->getTextForPersonality(personality);

            // Generate output path
            std::string responseType = std::to_string(static_cast<int>(pair.first));
            std::string personalityType = std::to_string(i);
            std::string voiceType = std::to_string(static_cast<int>(voice));

            std::string outputPath = "Audio/Dialogue/";
            outputPath += "response" + responseType + "_";
            outputPath += "personality" + personalityType + "_";
            outputPath += "voice" + voiceType + ".mp3";

            // Generate voice file
            if (generateVoiceWithElevenLabs(text, personality, voice, outputPath)) {
                response->setVoiceFilePath(personality, voice, outputPath);
            }
        }
    }
}

std::string DialogueManager::getVoiceIdFromType(VoiceType voice) {
    // Mock voice IDs
    switch (voice) {
    case VoiceType::Male1: return "male1";
    case VoiceType::Male2: return "male2";
    case VoiceType::Male3: return "male3";
    case VoiceType::Female1: return "female1";
    case VoiceType::Female2: return "female2";
    case VoiceType::Female3: return "female3";
    case VoiceType::Child1: return "child1";
    case VoiceType::Elderly1: return "elderly1";
    case VoiceType::Robot1: return "robot1";
    default: return "";
    }
}

std::string DialogueManager::getPromptForPersonality(ResponseType type, PersonalityType personality, const std::string& defaultText) {
    // Build a prompt for the GPT API based on response type and personality
    std::string prompt = "Create a dialogue response for a ";

    // Add personality
    switch (personality) {
    case PersonalityType::Bubbly: prompt += "bubbly"; break;
    case PersonalityType::Grumpy: prompt += "grumpy"; break;
    case PersonalityType::Manic: prompt += "manic"; break;
    case PersonalityType::Shy: prompt += "shy"; break;
    case PersonalityType::Serious: prompt += "serious"; break;
    case PersonalityType::Anxious: prompt += "anxious"; break;
    case PersonalityType::Confident: prompt += "confident"; break;
    case PersonalityType::Intellectual: prompt += "intellectual"; break;
    case PersonalityType::Mysterious: prompt += "mysterious"; break;
    case PersonalityType::Friendly: prompt += "friendly"; break;
    default: prompt += "generic"; break;
    }

    prompt += " character who is giving a ";

    // Add response type
    switch (type) {
    case ResponseType::EnthusiasticAffirmative: prompt += "very enthusiastic yes"; break;
    case ResponseType::IndifferentAffirmative: prompt += "indifferent yes"; break;
    case ResponseType::ReluctantAffirmative: prompt += "reluctant yes"; break;
    case ResponseType::EnthusiasticNegative: prompt += "very enthusiastic no"; break;
    case ResponseType::IndifferentNegative: prompt += "indifferent no"; break;
    case ResponseType::UntrustworthyResponse: prompt += "suspicious or untrustworthy answer"; break;
    case ResponseType::Confused: prompt += "confused response"; break;
    case ResponseType::Greeting: prompt += "greeting"; break;
    case ResponseType::Farewell: prompt += "farewell"; break;
    case ResponseType::MarkOnMap: prompt += "response about marking something on a map"; break;
    case ResponseType::GiveDirections: prompt += "response giving directions"; break;
    case ResponseType::AskForMoney: prompt += "response asking for money"; break;
    case ResponseType::OfferHelp: prompt += "response offering help"; break;
    case ResponseType::RefuseHelp: prompt += "response refusing to help"; break;
    case ResponseType::AskToFollow: prompt += "response asking someone to follow"; break;
    default: prompt += "general response"; break;
    }

    prompt += ".\n\nBase response: " + defaultText + "\n\nGenerated response:";

    return prompt;
}
