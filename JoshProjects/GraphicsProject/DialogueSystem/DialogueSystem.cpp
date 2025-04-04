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
    // Create mock personality variants if they don't exist
    if (m_personalityVariants.empty()) {
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

bool DialogueNode::updateChildCondition(int childId, const std::string& newCondition) {
    for (auto& childPair : m_children) {
        if (childPair.first->getId() == childId) {
            childPair.second = newCondition;
            return true;
        }
    }
    return false;
}

bool DialogueNode::removeChild(int childId) {
    for (auto it = m_children.begin(); it != m_children.end(); ++it) {
        if (it->first->getId() == childId) {
            m_children.erase(it);
            return true;
        }
    }
    return false;
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

    // Automatically generate variants for all personality types
    generatePersonalityVariants(response);

    return response;
}

void DialogueManager::createConditionBranches(std::shared_ptr<DialogueNode> conditionNode, int numBranches) {
    if (!conditionNode || conditionNode->getType() != DialogueNode::NodeType::ConditionCheck) {
        return; // Not a condition node
    }

    // Get the condition
    DialogueCondition condition = conditionNode->getCondition();

    // Get appropriate branch labels based on condition type
    std::vector<std::string> branchLabels = condition.GetBranchLabels();

    // Make sure we have at least the default branches
    while (branchLabels.size() < 2) {
        branchLabels.push_back("Branch " + std::to_string(branchLabels.size() + 1));
    }

    // Add more generic branches if needed
    while (branchLabels.size() < numBranches) {
        branchLabels.push_back("Branch " + std::to_string(branchLabels.size() + 1));
    }

    // Limit to requested number of branches
    branchLabels.resize(numBranches);

    // Get existing children and their conditions
    auto children = conditionNode->getChildren();
    std::vector<std::shared_ptr<DialogueNode>> existingNodes;
    std::vector<std::string> existingLabels;
    std::map<int, BranchCondition> existingBranchConditions;

    // Save existing children to preserve them
    for (int i = 0; i < children.size(); i++) {
        existingNodes.push_back(children[i].first);
        existingLabels.push_back(children[i].second);
        existingBranchConditions[i] = conditionNode->getBranchCondition(i);

        // Remove from the node but don't delete
        conditionNode->removeChild(children[i].first->getId());
    }

    // Determine how to reuse existing nodes
    int existingCount = static_cast<int>(existingNodes.size());
    int branchCount = static_cast<int>(branchLabels.size());

    // If we have specific condition type, create appropriate branch conditions
    if (condition.type == ConditionType::RelationshipStatus) {
        // For relationship conditions, create appropriate ranges
        RelationshipStatus threshold = condition.relationshipThreshold;

        // Set up default branch conditions
        std::vector<BranchCondition> defaultBranchConditions;

        switch (threshold) {
        case RelationshipStatus::Close:
            // If threshold is Close, branches are Close, Friendly, Neutral, Unfriendly, Hostile
            defaultBranchConditions.push_back(BranchCondition::CreateRelationshipRange(RelationshipStatus::Close, RelationshipStatus::Close));
            if (branchCount > 1) defaultBranchConditions.push_back(BranchCondition::CreateRelationshipRange(RelationshipStatus::Friendly, RelationshipStatus::Friendly));
            if (branchCount > 2) defaultBranchConditions.push_back(BranchCondition::CreateRelationshipRange(RelationshipStatus::Neutral, RelationshipStatus::Neutral));
            if (branchCount > 3) defaultBranchConditions.push_back(BranchCondition::CreateRelationshipRange(RelationshipStatus::Unfriendly, RelationshipStatus::Unfriendly));
            if (branchCount > 4) defaultBranchConditions.push_back(BranchCondition::CreateRelationshipRange(RelationshipStatus::Hostile, RelationshipStatus::Hostile));
            break;

        case RelationshipStatus::Friendly:
            // If threshold is Friendly, branches are Friendly+, Neutral, Unfriendly, Hostile
            defaultBranchConditions.push_back(BranchCondition::CreateRelationshipRange(RelationshipStatus::Friendly, RelationshipStatus::Close));
            if (branchCount > 1) defaultBranchConditions.push_back(BranchCondition::CreateRelationshipRange(RelationshipStatus::Neutral, RelationshipStatus::Neutral));
            if (branchCount > 2) defaultBranchConditions.push_back(BranchCondition::CreateRelationshipRange(RelationshipStatus::Unfriendly, RelationshipStatus::Unfriendly));
            if (branchCount > 3) defaultBranchConditions.push_back(BranchCondition::CreateRelationshipRange(RelationshipStatus::Hostile, RelationshipStatus::Hostile));
            break;

        case RelationshipStatus::Neutral:
            // If threshold is Neutral, branches are Neutral+, Unfriendly, Hostile
            defaultBranchConditions.push_back(BranchCondition::CreateRelationshipRange(RelationshipStatus::Neutral, RelationshipStatus::Close));
            if (branchCount > 1) defaultBranchConditions.push_back(BranchCondition::CreateRelationshipRange(RelationshipStatus::Unfriendly, RelationshipStatus::Unfriendly));
            if (branchCount > 2) defaultBranchConditions.push_back(BranchCondition::CreateRelationshipRange(RelationshipStatus::Hostile, RelationshipStatus::Hostile));
            break;

        case RelationshipStatus::Unfriendly:
            // If threshold is Unfriendly, branches are Unfriendly+, Hostile
            defaultBranchConditions.push_back(BranchCondition::CreateRelationshipRange(RelationshipStatus::Unfriendly, RelationshipStatus::Close));
            if (branchCount > 1) defaultBranchConditions.push_back(BranchCondition::CreateRelationshipRange(RelationshipStatus::Hostile, RelationshipStatus::Hostile));
            break;

        case RelationshipStatus::Hostile:
            // If threshold is Hostile, all relationships qualify
            defaultBranchConditions.push_back(BranchCondition::CreateRelationshipRange(RelationshipStatus::Hostile, RelationshipStatus::Close));
            break;
        }

        // Fill in any remaining branches with neutral conditions
        while (defaultBranchConditions.size() < branchCount) {
            defaultBranchConditions.push_back(BranchCondition());
        }

        // If we have fewer existing nodes than requested branches,
        // we'll reuse all existing ones and create new ones as needed
        if (existingCount <= branchCount) {
            // Reuse existing nodes
            for (int i = 0; i < existingCount; i++) {
                conditionNode->addChildNode(existingNodes[i], branchLabels[i]);

                // Keep existing branch condition if available, otherwise use default
                if (existingBranchConditions.find(i) != existingBranchConditions.end()) {
                    conditionNode->setBranchCondition(i, existingBranchConditions[i]);
                }
                else {
                    conditionNode->setBranchCondition(i, defaultBranchConditions[i]);
                }
            }

            // Create new nodes for any additional branches
            for (int i = existingCount; i < branchCount; i++) {
                auto childNode = createDialogueNode(
                    DialogueNode::NodeType::NPCStatement,
                    "Response for " + branchLabels[i]
                );
                conditionNode->addChildNode(childNode, branchLabels[i]);
                conditionNode->setBranchCondition(i, defaultBranchConditions[i]);
            }
        }
        // If we have more existing nodes than requested branches,
        // reuse the first N and let the rest be garbage collected
        else {
            // Reuse only the nodes we need
            for (int i = 0; i < branchCount; i++) {
                conditionNode->addChildNode(existingNodes[i], branchLabels[i]);

                // Keep existing branch condition if available, otherwise use default
                if (existingBranchConditions.find(i) != existingBranchConditions.end()) {
                    conditionNode->setBranchCondition(i, existingBranchConditions[i]);
                }
                else {
                    conditionNode->setBranchCondition(i, defaultBranchConditions[i]);
                }
            }
            // The rest of the existing nodes will be garbage collected
        }
    }
    else {
        // For other condition types, just reuse existing nodes or create new ones
        // without special branch conditions

        // If we have fewer existing nodes than requested branches,
        // we'll reuse all existing ones and create new ones as needed
        if (existingCount <= branchCount) {
            // Reuse existing nodes
            for (int i = 0; i < existingCount; i++) {
                conditionNode->addChildNode(existingNodes[i], branchLabels[i]);

                // Keep existing branch condition if available
                if (existingBranchConditions.find(i) != existingBranchConditions.end()) {
                    conditionNode->setBranchCondition(i, existingBranchConditions[i]);
                }
            }

            // Create new nodes for any additional branches
            for (int i = existingCount; i < branchCount; i++) {
                auto childNode = createDialogueNode(
                    DialogueNode::NodeType::NPCStatement,
                    "Response for " + branchLabels[i]
                );
                conditionNode->addChildNode(childNode, branchLabels[i]);
            }
        }
        // If we have more existing nodes than requested branches,
        // reuse the first N and let the rest be garbage collected
        else {
            // Reuse only the nodes we need
            for (int i = 0; i < branchCount; i++) {
                conditionNode->addChildNode(existingNodes[i], branchLabels[i]);

                // Keep existing branch condition if available
                if (existingBranchConditions.find(i) != existingBranchConditions.end()) {
                    conditionNode->setBranchCondition(i, existingBranchConditions[i]);
                }
            }
            // The rest of the existing nodes will be garbage collected
        }
    }
}

int DialogueManager::evaluateConditionBranchIndex(const DialogueCondition& condition, int npcId, int playerId) {
    // Default branch is 0 (first branch, success case)
    int branchIndex = 0;

    // Evaluate based on condition type
    switch (condition.type) {
    case ConditionType::RelationshipStatus: {
        // Get current relationship
        RelationshipStatus currentRelationship = getRelationshipStatus(npcId, playerId);

        // If relationship is below the threshold, use branch 1 (failure case)
        if (static_cast<int>(currentRelationship) < static_cast<int>(condition.relationshipThreshold)) {
            branchIndex = 1;
        }
        break;
    }

    case ConditionType::QuestStatus: {
        // Check if quest status matches
        bool statusMatches = checkQuestStatus(condition.parameterName, condition.parameterValue);
        if (!statusMatches) {
            branchIndex = 1; // Branch 1 for non-matching status
        }
        break;
    }

    case ConditionType::InventoryCheck: {
        // Parse quantity
        int quantity = 1;
        try {
            quantity = std::stoi(condition.parameterValue);
        }
        catch (...) {
            // Use default
        }

        // Check if player has enough items
        bool hasEnough = checkInventoryItem(condition.parameterName, quantity);
        if (!hasEnough) {
            branchIndex = 1; // Branch 1 for not enough items
        }
        break;
    }

    case ConditionType::StatCheck: {
        // Parse threshold
        int threshold = 1;
        try {
            threshold = std::stoi(condition.parameterValue);
        }
        catch (...) {
            // Use default
        }

        // Check if player stat meets threshold
        bool statHighEnough = checkPlayerStat(condition.parameterName, threshold);
        if (!statHighEnough) {
            branchIndex = 1; // Branch 1 for stat too low
        }
        break;
    }

    case ConditionType::TimeOfDay: {
        // This would need integration with your game's time system
        // For now, just use a random example
        std::string currentTimeOfDay = "Morning"; // This would come from your game

        if (currentTimeOfDay != condition.parameterName) {
            branchIndex = 1; // Branch 1 for time mismatch
        }
        break;
    }

    case ConditionType::Custom: {
        // Custom conditions would need their own evaluation logic
        // For now, let's assume a random 50/50 chance for testing
        if (rand() % 2 == 0) {
            branchIndex = 1; // Branch 1 with 50% chance
        }
        break;
    }

    default:
        // Default is branch 0
        break;
    }

    return branchIndex;
}

std::shared_ptr<DialogueNode> DialogueManager::getNodeByBranchIndex(std::shared_ptr<DialogueNode> conditionNode, int branchIndex) {
    if (!conditionNode) return nullptr;

    auto children = conditionNode->getChildren();
    if (children.empty()) return nullptr;

    // If branch index is out of bounds, use the last branch
    if (branchIndex < 0) branchIndex = 0;
    if (branchIndex >= static_cast<int>(children.size())) {
        branchIndex = static_cast<int>(children.size()) - 1;
    }

    // Find the node corresponding to the branch index
    int currentIndex = 0;
    for (const auto& childPair : children) {
        if (currentIndex == branchIndex) {
            return childPair.first;
        }
        currentIndex++;
    }

    // Fallback to first child if we couldn't find the right branch
    return children.front().first;
}

void DialogueManager::generatePersonalityVariants(std::shared_ptr<DialogueResponse> response) {
    // For each personality type, generate a variant
    for (int i = 0; i <= static_cast<int>(PersonalityType::Friendly); i++) {
        PersonalityType personality = static_cast<PersonalityType>(i);

        // Generate text with GPT (or use mock for now)
        std::string baseText = response->getTextForPersonality(PersonalityType::Bubbly);
        std::string generatedText = generateTextWithGPT(
            response->getType(), personality, baseText);

        // Set as unedited
        response->setTextForPersonality(personality, generatedText, false);

        std::cout << "Generated " << getPersonalityName(personality)
            << " variant: " << generatedText << std::endl;
    }
}

std::string DialogueManager::generateTextWithGPT(ResponseType type, PersonalityType personality, const std::string& defaultText) {
    if (m_gptApiKey.empty()) {
        std::cerr << "No GPT API key set. Using mock responses." << std::endl;

        // More detailed mock responses based on personality
        switch (personality) {
        case PersonalityType::Bubbly:
            return "Absolutely! I'd be super happy to help with that!";
        case PersonalityType::Grumpy:
            return "Yeah, fine, whatever. I'll do it.";
        case PersonalityType::Manic:
            return "YES! YES! ABSOLUTELY YES!! LET'S DO THIS RIGHT NOW!!";
        case PersonalityType::Shy:
            return "Um... I guess... if that's okay with you...";
        case PersonalityType::Serious:
            return "Affirmative. I will proceed with the task.";
        case PersonalityType::Anxious:
            return "Oh! Yes, I can do that... unless that's going to be a problem?";
        case PersonalityType::Confident:
            return "Of course I can. There's nobody better for the job.";
        case PersonalityType::Intellectual:
            return "Indeed, I shall undertake this task with methodical precision.";
        case PersonalityType::Mysterious:
            return "Perhaps... if fate allows it... the task shall be done.";
        case PersonalityType::Friendly:
            return "Sure thing, friend! Happy to help!";
        default:
            return defaultText + " [Mock response]";
        }
    }

    // In a real implementation, you would:
    // 1. Build a prompt with the personality and response type
    std::string prompt = getPromptForPersonality(type, personality, defaultText);

    // 2. Make an API call to GPT
    // 3. Parse the response

    // For demo, just return the mock
    return defaultText + " [API Key present but using mock]";
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

void DialogueManager::generateAllVoiceVariants(VoiceType defaultVoice) {
    // For each response type
    for (auto& pair : m_responses) {
        auto& response = pair.second;

        // For each personality type
        for (int i = 0; i <= static_cast<int>(PersonalityType::Friendly); i++) {
            PersonalityType personality = static_cast<PersonalityType>(i);
            std::string text = response->getTextForPersonality(personality);

            // Generate voice for the default voice type
            std::string outputPath = buildAudioFilePath(pair.first, personality, defaultVoice);

            // Generate voice file if it doesn't exist already
            if (!response->hasVoiceGenerated(personality, defaultVoice)) {
                if (generateVoiceWithElevenLabs(text, personality, defaultVoice, outputPath)) {
                    response->setVoiceFilePath(personality, defaultVoice, outputPath);
                    std::cout << "Generated voice for " << getResponseTypeName(pair.first)
                        << " with " << getPersonalityName(personality)
                        << " personality and " << getVoiceTypeName(defaultVoice) << " voice." << std::endl;
                }
            }
        }
    }
}

std::string DialogueManager::getPersonalityName(PersonalityType type) const {
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

std::string DialogueManager::getResponseTypeName(ResponseType type) const {
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

std::string DialogueManager::getVoiceTypeName(VoiceType type) const {
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

RelationshipStatus DialogueManager::getRelationshipStatus(int npcId, int playerId) {
    // Check the test relationship map
    auto key = std::make_pair(npcId, playerId);
    auto it = m_testRelationships.find(key);

    if (it != m_testRelationships.end()) {
        return it->second;
    }

    // Default is neutral
    return RelationshipStatus::Neutral;
}

bool DialogueManager::registerWithWwise() {
    if (!m_audioEngine) {
        std::cerr << "Error: Audio engine not initialized" << std::endl;
        return false;
    }

    // For each response type
    for (auto& pair : m_responses) {
        auto& response = pair.second;
        ResponseType type = pair.first;

        // For each personality/voice combination
        for (int i = 0; i <= static_cast<int>(PersonalityType::Friendly); i++) {
            PersonalityType personality = static_cast<PersonalityType>(i);

            for (int j = 0; j <= static_cast<int>(VoiceType::Robot1); j++) {
                VoiceType voice = static_cast<VoiceType>(j);

                if (response->hasVoiceGenerated(personality, voice)) {
                    std::string filePath = response->getVoiceFilePath(personality, voice);

                    // Create a unique event ID based on the response type, personality and voice
                    AkUniqueID eventID = createDialogueEventID(type, personality, voice);

                    // Register the audio file with Wwise (mock implementation)
                    std::cout << "Registering dialogue audio with Wwise: " << filePath
                        << " as event ID " << eventID << std::endl;

                    // In a real implementation, you would:
                    // 1. Register the file with Wwise
                    // 2. Create an event for it
                    // 3. Store the mapping for later use
                }
            }
        }
    }

    return true;
}

DialogueManager::AkUniqueID DialogueManager::createDialogueEventID(ResponseType type, PersonalityType personality, VoiceType voice) {
    // This is a simple hash function to create a unique ID
    // In reality, you would use Wwise's event ID system
    uint32_t typeVal = static_cast<uint32_t>(type);
    uint32_t personalityVal = static_cast<uint32_t>(personality);
    uint32_t voiceVal = static_cast<uint32_t>(voice);

    return 1000000 + (typeVal * 10000) + (personalityVal * 100) + voiceVal;
}

bool DialogueManager::playDialogueResponse(ResponseType type, PersonalityType personality, VoiceType voice) {
    if (!m_audioEngine) {
        std::cerr << "Error: Audio engine not initialized" << std::endl;
        return false;
    }

    // Get the response
    auto response = getResponse(type);
    if (!response) {
        std::cerr << "Error: Response not found for type " << static_cast<int>(type) << std::endl;
        return false;
    }

    // Check if voice is generated
    if (!response->hasVoiceGenerated(personality, voice)) {
        std::cerr << "Error: Voice not generated for this combination" << std::endl;
        return false;
    }

    // Get file path
    std::string filePath = response->getVoiceFilePath(personality, voice);

    // Create event ID
    AkUniqueID eventID = createDialogueEventID(type, personality, voice);

    // In a real implementation, you would:
    // 1. Play the Wwise event
    std::cout << "Playing dialogue event ID " << eventID << " from file " << filePath << std::endl;

    return true;
}

std::vector<std::shared_ptr<DialogueNode>> DialogueManager::getAllNodes() const {
    std::vector<std::shared_ptr<DialogueNode>> result;
    for (const auto& pair : m_nodes) {
        result.push_back(pair.second);
    }
    return result;
}

bool DialogueManager::deleteNode(int nodeId) {
    // First check if the node exists
    auto it = m_nodes.find(nodeId);
    if (it == m_nodes.end()) {
        return false;
    }

    // Get the node to delete
    auto nodeToDelete = it->second;

    // First, recursively delete all child nodes
    auto children = nodeToDelete->getChildren();
    for (const auto& childPair : children) {
        deleteNode(childPair.first->getId());
    }

    // Remove this node from any parent nodes
    removeChildFromParent(nodeId);

    // Remove this node from our map
    m_nodes.erase(it);

    return true;
}

std::shared_ptr<DialogueNode> DialogueManager::duplicateNode(int nodeId) {
    auto sourceNode = getNodeById(nodeId);
    if (!sourceNode) {
        return nullptr;
    }

    // Create a new node with the same type and text
    auto newNode = createDialogueNode(sourceNode->getType(), sourceNode->getText() + " (Copy)");

    // Copy the response if any
    if (auto response = sourceNode->getResponse()) {
        newNode->setResponse(response);
    }

    // Copy the condition if relevant
    newNode->setCondition(sourceNode->getCondition());

    // We don't copy children, as that would potentially create a deep copy

    return newNode;
}

bool DialogueManager::hasParent(int nodeId) const {
    // Check if this node is a child of any other node
    return findParentNode(nodeId) != nullptr;
}

std::shared_ptr<DialogueNode> DialogueManager::findParentNode(int childId) const {
    for (const auto& pair : m_nodes) {
        auto& parentNode = pair.second;
        auto children = parentNode->getChildren();

        for (const auto& childPair : children) {
            if (childPair.first->getId() == childId) {
                return parentNode;
            }
        }
    }
    return nullptr;
}

bool DialogueManager::removeChildFromParent(int childId) {
    auto parent = findParentNode(childId);
    if (parent) {
        return parent->removeChild(childId);
    }
    return false;
}

bool DialogueManager::updateNodeType(int nodeId, DialogueNode::NodeType newType) {
    auto it = m_nodes.find(nodeId);
    if (it == m_nodes.end()) {
        return false;
    }

    // Since we can't modify the type directly (it's private),
    // we need to create a new node and transfer properties
    auto oldNode = it->second;
    auto newNode = createDialogueNode(newType, oldNode->getText());

    // Copy response if present
    auto response = oldNode->getResponse();
    if (response) {
        newNode->setResponse(response);
    }

    // Copy condition if present
    newNode->setCondition(oldNode->getCondition());

    // Copy children
    for (const auto& childPair : oldNode->getChildren()) {
        newNode->addChildNode(childPair.first, childPair.second);
    }

    // Replace the old node in all parent nodes
    for (auto& pair : m_nodes) {
        auto& node = pair.second;
        auto children = node->getChildren();

        for (const auto& childPair : children) {
            if (childPair.first->getId() == nodeId) {
                // Replace child with new node
                node->removeChild(nodeId);
                node->addChildNode(newNode, childPair.second);
            }
        }
    }

    // Replace old node in the map
    m_nodes[nodeId] = newNode;

    return true;
}

bool DialogueManager::evaluateBranchCondition(const BranchCondition& branchCond, int npcId, int playerId) {
    // Evaluate based on condition type
    switch (branchCond.type) {
    case ConditionType::RelationshipStatus: {
        // Get current relationship
        RelationshipStatus currentRelationship = getRelationshipStatus(npcId, playerId);

        // Check if relationship is within the specified range
        return static_cast<int>(currentRelationship) >= static_cast<int>(branchCond.minRelationship) &&
            static_cast<int>(currentRelationship) <= static_cast<int>(branchCond.maxRelationship);
    }

    case ConditionType::InventoryCheck: {
        // Check if player has item quantity within range
        int quantity = 0; // This would come from your game's inventory system

        // For testing, use a mock inventory
        auto it = m_testInventory.find(branchCond.itemId);
        if (it != m_testInventory.end()) {
            quantity = it->second;
        }

        return quantity >= branchCond.minQuantity && quantity <= branchCond.maxQuantity;
    }

    case ConditionType::StatCheck: {
        // Check if player stat is within range
        int statValue = 0; // This would come from your game's stat system

        // For testing, use a mock stat system
        auto it = m_testPlayerStats.find(branchCond.statName);
        if (it != m_testPlayerStats.end()) {
            statValue = it->second;
        }

        return statValue >= branchCond.minValue && statValue <= branchCond.maxValue;
    }

    case ConditionType::QuestStatus: {
        // Check if quest has the specified status
        std::string currentStatus = "Not Started"; // Default for non-existing quests

        // For testing, use a mock quest system
        auto it = m_testQuestStates.find(branchCond.questId);
        if (it != m_testQuestStates.end()) {
            currentStatus = it->second;
        }

        return currentStatus == branchCond.questStatus;
    }

    case ConditionType::TimeOfDay: {
        // Check if current time matches specified time
        std::string currentTime = "Morning"; // This would come from your game's time system

        // For testing, use a mock time ("Morning", "Afternoon", "Evening", "Night")
        // You would replace this with your actual game time system

        return currentTime == branchCond.timeOfDay;
    }

    case ConditionType::Custom: {
        // For custom conditions, you would implement game-specific logic
        // For testing, return true for "True" and false for anything else
        return branchCond.parameterValue == "True";
    }

    default:
        return false;
    }
}

std::shared_ptr<DialogueNode> DialogueManager::findNextNode(std::shared_ptr<DialogueNode> currentNode, int npcId, int playerId) {
    if (!currentNode) {
        return nullptr;
    }

    // Handle based on node type
    switch (currentNode->getType()) {
    case DialogueNode::NodeType::ConditionCheck: {
        // Get all child branches
        auto children = currentNode->getChildren();
        if (children.empty()) {
            return nullptr; // No branches to follow
        }

        // For each branch, check if its condition is met
        for (int i = 0; i < children.size(); i++) {
            // Get branch-specific condition
            BranchCondition branchCond = currentNode->getBranchCondition(i);

            // If not properly initialized, use the main condition
            if (branchCond.type == ConditionType::None) {
                // Evaluate with the main condition instead
                if (evaluateCondition(currentNode->getCondition(), npcId, playerId)) {
                    // This is the success branch
                    if (i == 0) {
                        return children[i].first;
                    }
                }
                else {
                    // This is the failure branch
                    if (i == 1 && children.size() > 1) {
                        return children[i].first;
                    }
                }
            }
            else {
                // Evaluate branch-specific condition
                if (evaluateBranchCondition(branchCond, npcId, playerId)) {
                    return children[i].first;
                }
            }
        }

        // If no branch condition is met, use the first branch as default
        return children[0].first;
    }

    case DialogueNode::NodeType::BranchPoint: {
        // For branch points, let the caller decide which branch to take
        // Just return the node itself
        return currentNode;
    }

    default: {
        // For other node types, just return the node itself
        return currentNode;
    }
    }
}

bool DialogueManager::evaluateCondition(const DialogueCondition& condition, int npcId, int playerId) {
    // Based on condition type, evaluate it
    switch (condition.type) {
    case ConditionType::None:
        return true; // No condition always passes

    case ConditionType::RelationshipStatus: {
        RelationshipStatus npcRelationship = getRelationshipStatus(npcId, playerId);
        return static_cast<int>(npcRelationship) >= static_cast<int>(condition.relationshipThreshold);
    }

    case ConditionType::QuestStatus:
        return checkQuestStatus(condition.parameterName, condition.parameterValue);

    case ConditionType::InventoryCheck: {
        int quantity = 0;
        try {
            quantity = std::stoi(condition.parameterValue);
        }
        catch (...) {
            return false; // Invalid quantity
        }
        return checkInventoryItem(condition.parameterName, quantity);
    }

    case ConditionType::StatCheck: {
        int threshold = 0;
        try {
            threshold = std::stoi(condition.parameterValue);
        }
        catch (...) {
            return false; // Invalid threshold
        }
        return checkPlayerStat(condition.parameterName, threshold);
    }

    case ConditionType::Custom:
        // For custom conditions, we'd need a scripting system
        // For now, assume custom conditions pass for testing
        return true;

    default:
        return false; // Unknown condition type fails
    }
}

bool DialogueManager::checkQuestStatus(const std::string& questId, const std::string& status) {
    // Check the test quest state map
    auto it = m_testQuestStates.find(questId);

    if (it != m_testQuestStates.end()) {
        return it->second == status;
    }

    // Default is quest not found
    return false;
}

bool DialogueManager::checkInventoryItem(const std::string& itemId, int quantity) {
    // Check the test inventory map
    auto it = m_testInventory.find(itemId);

    if (it != m_testInventory.end()) {
        return it->second >= quantity;
    }

    // Default is item not in inventory
    return false;
}

bool DialogueManager::checkPlayerStat(const std::string& statName, int threshold) {
    // Check the test player stats map
    auto it = m_testPlayerStats.find(statName);

    if (it != m_testPlayerStats.end()) {
        return it->second >= threshold;
    }

    // Default is stat not found
    return false;
}

void DialogueManager::clearAllNodes() {
    // Get a copy of all node IDs since we'll be modifying the collection
    std::vector<int> nodeIds;
    for (const auto& pair : m_nodes) {
        nodeIds.push_back(pair.first);
    }

    // Delete nodes in reverse order (children before parents)
    for (auto it = nodeIds.rbegin(); it != nodeIds.rend(); ++it) {
        m_nodes.erase(*it);
    }
}

std::string DialogueManager::buildAudioFilePath(ResponseType type, PersonalityType personality, VoiceType voice) {
    std::string responseType = std::to_string(static_cast<int>(type));
    std::string personalityType = std::to_string(static_cast<int>(personality));
    std::string voiceType = std::to_string(static_cast<int>(voice));

    std::string outputPath = "Audio/Dialogue/";
    outputPath += "response" + responseType + "_";
    outputPath += "personality" + personalityType + "_";
    outputPath += "voice" + voiceType + ".mp3";

    return outputPath;
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

void DialogueManager::setRelationshipForTesting(int npcId, int playerId, RelationshipStatus status) {
    m_testRelationships[std::make_pair(npcId, playerId)] = status;
}
