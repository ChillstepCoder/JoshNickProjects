//DialogueSystem.h

#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <functional>
#include <unordered_map>
#include "JAGEngine/IMainGame.h"

// Forward declaration of Wwise Audio Engine from JAGEngine
namespace JAGEngine {
    class WWiseAudioEngine;
}

// ==========================================================
// Dialogue System Core Classes
// ==========================================================

// Personality types for NPCs
enum class PersonalityType {
    Bubbly,
    Grumpy,
    Manic,
    Shy,
    Serious,
    Anxious,
    Confident,
    Intellectual,
    Mysterious,
    Friendly
    // Add more as needed
};

// Response types categorize the kind of response
enum class ResponseType {
    EnthusiasticAffirmative,
    IndifferentAffirmative,
    ReluctantAffirmative,
    EnthusiasticNegative,
    IndifferentNegative,
    UntrustworthyResponse,
    Confused,
    Greeting,
    Farewell,
    MarkOnMap,
    GiveDirections,
    AskForMoney,
    OfferHelp,
    RefuseHelp,
    AskToFollow,
    // Add more as needed
};

// Voice types correspond to the ElevenLabs voices we'll use
enum class VoiceType {
    Male1,
    Male2,
    Male3,
    Female1,
    Female2,
    Female3,
    Child1,
    Elderly1,
    Robot1
    // Add more as needed
};

enum class ConditionType {
    None,                  // No condition
    RelationshipStatus,    // Based on NPC's feeling about player
    QuestStatus,           // Based on quest completion status 
    InventoryCheck,        // Player has certain items
    StatCheck,             // Player stat check (e.g., strength > 5)
    TimeOfDay,             // Game world time condition
    PlayerChoice,          // Based on previous player choices
    Custom                 // Custom script condition
};

enum class RelationshipStatus {
    Hostile,       // NPC dislikes player strongly
    Unfriendly,    // NPC dislikes player
    Neutral,       // NPC has no strong feelings
    Friendly,      // NPC likes player
    Close          // NPC likes player strongly
};

struct BranchCondition {
    ConditionType type = ConditionType::None;

    // Relationship range parameters
    RelationshipStatus minRelationship = RelationshipStatus::Neutral;
    RelationshipStatus maxRelationship = RelationshipStatus::Close;

    // Inventory check parameters
    std::string itemId = "";
    int minQuantity = 0;
    int maxQuantity = 9999;

    // Quest status parameters
    std::string questId = "";
    std::string questStatus = "";

    // Stat check parameters
    std::string statName = "";
    int minValue = 0;
    int maxValue = 9999;

    // Time of day parameters
    std::string timeOfDay = "";

    // General parameters
    std::string parameterName = "";
    std::string parameterValue = "";
    std::string description = "";

    // Helper methods for creating different condition types

    static BranchCondition CreateRelationshipRange(RelationshipStatus minRel, RelationshipStatus maxRel) {
        BranchCondition condition;
        condition.type = ConditionType::RelationshipStatus;
        condition.minRelationship = minRel;
        condition.maxRelationship = maxRel;
        condition.description = GetRelationshipRangeDescription(minRel, maxRel);
        return condition;
    }

    static BranchCondition CreateInventoryRange(const std::string& itemId, int minQty, int maxQty) {
        BranchCondition condition;
        condition.type = ConditionType::InventoryCheck;
        condition.itemId = itemId;
        condition.minQuantity = minQty;
        condition.maxQuantity = maxQty;
        condition.description = GetInventoryRangeDescription(itemId, minQty, maxQty);
        return condition;
    }

    static BranchCondition CreateQuestStatus(const std::string& questId, const std::string& status) {
        BranchCondition condition;
        condition.type = ConditionType::QuestStatus;
        condition.questId = questId;
        condition.questStatus = status;
        condition.description = "Quest '" + questId + "' is " + status;
        return condition;
    }

    static BranchCondition CreateStatRange(const std::string& statName, int minVal, int maxVal) {
        BranchCondition condition;
        condition.type = ConditionType::StatCheck;
        condition.statName = statName;
        condition.minValue = minVal;
        condition.maxValue = maxVal;
        condition.description = GetStatRangeDescription(statName, minVal, maxVal);
        return condition;
    }

    static BranchCondition CreateTimeOfDay(const std::string& timeOfDay) {
        BranchCondition condition;
        condition.type = ConditionType::TimeOfDay;
        condition.timeOfDay = timeOfDay;
        condition.description = "Time is " + timeOfDay;
        return condition;
    }

    static BranchCondition CreateCustomCondition(const std::string& name, const std::string& value) {
        BranchCondition condition;
        condition.type = ConditionType::Custom;
        condition.parameterName = name;
        condition.parameterValue = value;
        condition.description = "Custom: " + name + " = " + value;
        return condition;
    }

    // Description helper methods

    static std::string GetRelationshipRangeDescription(RelationshipStatus minRel, RelationshipStatus maxRel) {
        std::string minName = GetRelationshipName(minRel);
        std::string maxName = GetRelationshipName(maxRel);

        if (minRel == maxRel) {
            return "Relationship = " + minName;
        }
        else {
            return "Relationship: " + minName + " to " + maxName;
        }
    }

    static std::string GetInventoryRangeDescription(const std::string& itemId, int minQty, int maxQty) {
        if (minQty == maxQty) {
            return "Has exactly " + std::to_string(minQty) + " " + itemId;
        }
        else if (maxQty == 9999) {
            return "Has " + std::to_string(minQty) + "+ " + itemId;
        }
        else if (minQty == 0) {
            return "Has 0-" + std::to_string(maxQty) + " " + itemId;
        }
        else {
            return "Has " + std::to_string(minQty) + "-" + std::to_string(maxQty) + " " + itemId;
        }
    }

    static std::string GetStatRangeDescription(const std::string& statName, int minVal, int maxVal) {
        if (minVal == maxVal) {
            return statName + " = " + std::to_string(minVal);
        }
        else if (maxVal == 9999) {
            return statName + " ? " + std::to_string(minVal);
        }
        else if (minVal == 0) {
            return statName + " ? " + std::to_string(maxVal);
        }
        else {
            return statName + ": " + std::to_string(minVal) + "-" + std::to_string(maxVal);
        }
    }

    static std::string GetRelationshipName(RelationshipStatus status) {
        switch (status) {
        case RelationshipStatus::Hostile: return "Hostile";
        case RelationshipStatus::Unfriendly: return "Unfriendly";
        case RelationshipStatus::Neutral: return "Neutral";
        case RelationshipStatus::Friendly: return "Friendly";
        case RelationshipStatus::Close: return "Close";
        default: return "Unknown";
        }
    }
};

struct DialogueCondition {
    ConditionType type = ConditionType::None;
    std::string parameterName = "";
    std::string parameterValue = "";
    RelationshipStatus relationshipThreshold = RelationshipStatus::Neutral;

    // Helper for relationship conditions
    static DialogueCondition CreateRelationshipCondition(RelationshipStatus threshold) {
        DialogueCondition condition;
        condition.type = ConditionType::RelationshipStatus;
        condition.relationshipThreshold = threshold;
        return condition;
    }

    // Helper for quest conditions
    static DialogueCondition CreateQuestCondition(const std::string& questId, const std::string& status) {
        DialogueCondition condition;
        condition.type = ConditionType::QuestStatus;
        condition.parameterName = questId;
        condition.parameterValue = status;
        return condition;
    }

    // Helper for inventory conditions
    static DialogueCondition CreateInventoryCondition(const std::string& itemId, const std::string& quantity) {
        DialogueCondition condition;
        condition.type = ConditionType::InventoryCheck;
        condition.parameterName = itemId;
        condition.parameterValue = quantity;
        return condition;
    }

    // Helper for stat check conditions
    static DialogueCondition CreateStatCheckCondition(const std::string& statName, const std::string& threshold) {
        DialogueCondition condition;
        condition.type = ConditionType::StatCheck;
        condition.parameterName = statName;
        condition.parameterValue = threshold;
        return condition;
    }

    // Helper for time of day conditions
    static DialogueCondition CreateTimeOfDayCondition(const std::string& timeOfDay) {
        DialogueCondition condition;
        condition.type = ConditionType::TimeOfDay;
        condition.parameterName = timeOfDay;
        return condition;
    }

    // Helper for player choice conditions
    static DialogueCondition CreatePlayerChoiceCondition(const std::string& choiceId) {
        DialogueCondition condition;
        condition.type = ConditionType::PlayerChoice;
        condition.parameterName = choiceId;
        return condition;
    }

    // Helper for custom conditions
    static DialogueCondition CreateCustomCondition(const std::string& conditionName, const std::string& conditionValue) {
        DialogueCondition condition;
        condition.type = ConditionType::Custom;
        condition.parameterName = conditionName;
        condition.parameterValue = conditionValue;
        return condition;
    }

    std::string GetDescription() const {
        switch (type) {
        case ConditionType::None:
            return "No condition";
        case ConditionType::RelationshipStatus:
            return "Relationship >= " + GetRelationshipName(relationshipThreshold);
        case ConditionType::QuestStatus:
            return "Quest '" + parameterName + "' is " + parameterValue;
        case ConditionType::InventoryCheck:
            return "Has item '" + parameterName + "' x" + parameterValue;
        case ConditionType::StatCheck:
            return "Stat '" + parameterName + "' >= " + parameterValue;
        case ConditionType::TimeOfDay:
            return "Time is " + parameterName;
        case ConditionType::PlayerChoice:
            return "Player chose '" + parameterName + "'";
        case ConditionType::Custom:
            return "Custom: " + parameterName + " = " + parameterValue;
        default:
            return "Unknown condition";
        }
    }

    std::vector<std::string> GetBranchLabels() const {
        std::vector<std::string> labels;

        switch (type) {
        case ConditionType::RelationshipStatus: {
            // Create labels based on relationship threshold
            switch (relationshipThreshold) {
            case RelationshipStatus::Close:
                labels.push_back("Close");
                labels.push_back("Less than Close");
                break;
            case RelationshipStatus::Friendly:
                labels.push_back("Friendly+");
                labels.push_back("Unfriendly/Neutral");
                break;
            case RelationshipStatus::Neutral:
                labels.push_back("Neutral+");
                labels.push_back("Hostile/Unfriendly");
                break;
            case RelationshipStatus::Unfriendly:
                labels.push_back("Unfriendly+");
                labels.push_back("Hostile");
                break;
            case RelationshipStatus::Hostile:
                labels.push_back("Hostile");
                labels.push_back("None"); // Impossible condition
                break;
            default:
                labels.push_back("Success");
                labels.push_back("Failure");
                break;
            }
            break;
        }

        case ConditionType::TimeOfDay: {
            // For time of day, we can have morning, afternoon, evening, night
            if (parameterName == "Morning") {
                labels.push_back("Morning");
                labels.push_back("Not Morning");
            }
            else if (parameterName == "Afternoon") {
                labels.push_back("Afternoon");
                labels.push_back("Not Afternoon");
            }
            else if (parameterName == "Evening") {
                labels.push_back("Evening");
                labels.push_back("Not Evening");
            }
            else if (parameterName == "Night") {
                labels.push_back("Night");
                labels.push_back("Not Night");
            }
            else {
                labels.push_back("Time Match");
                labels.push_back("Time Mismatch");
            }
            break;
        }

        case ConditionType::QuestStatus: {
            // For quest status, we can have different states
            labels.push_back(parameterValue); // The expected state
            labels.push_back("Other States"); // All other states
            break;
        }

        case ConditionType::InventoryCheck: {
            // For inventory checks, has/doesn't have
            labels.push_back("Has Item");
            labels.push_back("Doesn't Have Item");
            break;
        }

        case ConditionType::StatCheck: {
            // For stat checks, passes/fails
            labels.push_back("Stat ? " + parameterValue);
            labels.push_back("Stat < " + parameterValue);
            break;
        }

        case ConditionType::PlayerChoice: {
            // For player choice, selected/not selected
            labels.push_back("Selected");
            labels.push_back("Not Selected");
            break;
        }

        case ConditionType::Custom: {
            // For custom conditions, true/false or custom labels
            if (!parameterValue.empty()) {
                // Use parameter value as success label if provided
                labels.push_back(parameterValue);
                labels.push_back("Not " + parameterValue);
            }
            else {
                labels.push_back("True");
                labels.push_back("False");
            }
            break;
        }

        default:
            // Default case for None or unknown types
            labels.push_back("Success");
            labels.push_back("Failure");
            break;
        }

        return labels;
    }

private:
    std::string GetRelationshipName(RelationshipStatus status) const {
        switch (status) {
        case RelationshipStatus::Hostile: return "Hostile";
        case RelationshipStatus::Unfriendly: return "Unfriendly";
        case RelationshipStatus::Neutral: return "Neutral";
        case RelationshipStatus::Friendly: return "Friendly";
        case RelationshipStatus::Close: return "Close";
        default: return "Unknown";
        }
    }
};

// Forward declarations
class DialogueNode;
class DialogueManager;
class DialogueResponse;

// Represents a single line of dialogue with variations by personality
class DialogueResponse {
public:
    DialogueResponse(ResponseType type, const std::string& defaultText);
    ~DialogueResponse() = default;

    // Core properties
    ResponseType getType() const { return m_type; }
    std::string getName() const { return m_name; }
    void setName(const std::string& name) { m_name = name; }

    // Text variations
    std::string getTextForPersonality(PersonalityType personality) const;
    void setTextForPersonality(PersonalityType personality, const std::string& text, bool isEdited = false);
    bool isTextEdited(PersonalityType personality) const;

    // Voice generation
    bool hasVoiceGenerated(PersonalityType personality, VoiceType voice) const;
    std::string getVoiceFilePath(PersonalityType personality, VoiceType voice) const;
    void setVoiceFilePath(PersonalityType personality, VoiceType voice, const std::string& path);

    // Mock API for now
    void generateAllPersonalityVariants();
    void regeneratePersonalityVariant(PersonalityType personality);

private:
    ResponseType m_type;
    std::string m_name;
    std::string m_defaultText;
    std::map<PersonalityType, std::string> m_personalityVariants;
    std::map<PersonalityType, bool> m_isEdited;
    std::map<std::pair<PersonalityType, VoiceType>, std::string> m_voiceFilePaths;
};

// Represents a dialogue tree node (question, statement, or choice)
class DialogueNode {
public:
    enum class NodeType {
        NPCStatement,           // Simple NPC statement
        PlayerChoice,        // Player selects from options
        DynamicStatement,    // Statement with variables
        ConditionCheck,      // Checks a condition (like relationship status)
        BranchPoint          // Pure branching point with no displayed text
    };

    DialogueNode(NodeType type, const std::string& text);
    ~DialogueNode() = default;

    // Node properties
    NodeType getType() const { return m_type; }
    std::string getText() const { return m_text; }
    void setText(const std::string& text) { m_text = text; }
    int getId() const { return m_id; }

    // Child nodes
    void addChildNode(std::shared_ptr<DialogueNode> node, const std::string& condition = "");
    std::vector<std::pair<std::shared_ptr<DialogueNode>, std::string>> getChildren() const;
    bool updateChildCondition(int childId, const std::string& newCondition);
    bool removeChild(int childId);

    // Response connections
    void setResponse(std::shared_ptr<DialogueResponse> response);
    std::shared_ptr<DialogueResponse> getResponse() const;

    void setCondition(const DialogueCondition& condition) { m_condition = condition; }
    DialogueCondition getCondition() const { return m_condition; }

    void setBranchCondition(int branchIndex, const BranchCondition& condition) {
        m_branchConditions[branchIndex] = condition;
    }

    BranchCondition getBranchCondition(int branchIndex) const {
        auto it = m_branchConditions.find(branchIndex);
        if (it != m_branchConditions.end()) {
            return it->second;
        }

        // Return default condition if not found
        return BranchCondition();
    }

private:
    static int s_nextId;

    int m_id;
    NodeType m_type;
    std::string m_text;
    std::vector<std::pair<std::shared_ptr<DialogueNode>, std::string>> m_children;
    std::shared_ptr<DialogueResponse> m_response;
    DialogueCondition m_condition;
    std::map<int, BranchCondition> m_branchConditions;
};

// Manages all dialogue responses and provides access to APIs
class DialogueManager {
public:
    DialogueManager();
    ~DialogueManager();

    // Initialization
    bool initialize(JAGEngine::WWiseAudioEngine* audioEngine);
    void shutdown();

    using AkUniqueID = unsigned int;

    // Dialogue response management
    std::shared_ptr<DialogueResponse> createResponse(ResponseType type, const std::string& defaultText);
    void createConditionBranches(std::shared_ptr<DialogueNode> conditionNode, int numBranches);
    int evaluateConditionBranchIndex(const DialogueCondition& condition, int npcId, int playerId);
    std::shared_ptr<DialogueNode> getNodeByBranchIndex(std::shared_ptr<DialogueNode> conditionNode, int branchIndex);
    std::shared_ptr<DialogueResponse> getResponse(ResponseType type);
    std::vector<std::shared_ptr<DialogueResponse>> getAllResponses() const;

    // Dialogue tree management
    std::shared_ptr<DialogueNode> createDialogueNode(DialogueNode::NodeType type, const std::string& text);
    std::shared_ptr<DialogueNode> getNodeById(int id);

    // API integrations
    void setAPIKeys(const std::string& gptKey, const std::string& elevenLabsKey);
    std::string generateTextWithGPT(ResponseType type, PersonalityType personality, const std::string& defaultText);
    bool generateVoiceWithElevenLabs(const std::string& text, PersonalityType personality, VoiceType voice, const std::string& outputPath);

    // Batch operations
    void generateAllTextVariants();
    void generateAllVoiceVariants(VoiceType defaultVoice = VoiceType::Male1);
    void generatePersonalityVariants(std::shared_ptr<DialogueResponse> response);

    // Wwise integration
    using AkUniqueID = unsigned int; // Define AkUniqueID as a typedef
    bool registerWithWwise();
    bool playDialogueResponse(ResponseType type, PersonalityType personality, VoiceType voice);

    // dialogue tree editor
    std::vector<std::shared_ptr<DialogueNode>> getAllNodes() const;
    bool deleteNode(int nodeId);
    std::shared_ptr<DialogueNode> duplicateNode(int nodeId);
    bool hasParent(int nodeId) const;
    bool updateNodeType(int nodeId, DialogueNode::NodeType newType);
    bool evaluateBranchCondition(const BranchCondition& branchCond, int npcId, int playerId);
    std::shared_ptr<DialogueNode> findNextNode(std::shared_ptr<DialogueNode> currentNode, int npcId, int playerId);

    bool evaluateCondition(const DialogueCondition& condition, int npcId, int playerId);
    bool checkQuestStatus(const std::string& questId, const std::string& status);
    bool checkInventoryItem(const std::string& itemId, int quantity);
    bool checkPlayerStat(const std::string& statName, int threshold);
    void clearAllNodes();


    // Helper methods
    std::string buildAudioFilePath(ResponseType type, PersonalityType personality, VoiceType voice);
    AkUniqueID createDialogueEventID(ResponseType type, PersonalityType personality, VoiceType voice);

    std::shared_ptr<DialogueNode> findParentNode(int childId) const;
    bool removeChildFromParent(int childId);

    void setGame(JAGEngine::IMainGame* game) { m_game = game; }
    JAGEngine::IMainGame* getGame() const { return m_game; }

    // Helper methods for DialogueManager class
    std::string getPersonalityName(PersonalityType type) const;
    std::string getResponseTypeName(ResponseType type) const;
    std::string getVoiceTypeName(VoiceType type) const;
    RelationshipStatus getRelationshipStatus(int npcId, int playerId);
    void setInventoryItemForTesting(const std::string& itemId, int quantity) {
        m_testInventory[itemId] = quantity;
    }

    void setPlayerStatForTesting(const std::string& statName, int value) {
        m_testPlayerStats[statName] = value;
    }

    void setQuestStatusForTesting(const std::string& questId, const std::string& status) {
        m_testQuestStates[questId] = status;
    }

    void setTimeOfDayForTesting(const std::string& timeOfDay) {
        m_testTimeOfDay = timeOfDay;
    }


    //test methods
    void setRelationshipForTesting(int npcId, int playerId, RelationshipStatus status);

private:
    JAGEngine::IMainGame* m_game;
    JAGEngine::WWiseAudioEngine* m_audioEngine;
    std::string m_gptApiKey;
    std::string m_elevenLabsKey;
    std::string m_elevenLabsApiKey;
    std::unordered_map<ResponseType, std::shared_ptr<DialogueResponse>> m_responses;
    std::unordered_map<int, std::shared_ptr<DialogueNode>> m_nodes;

    // Helper methods
    std::string getPromptForPersonality(ResponseType type, PersonalityType personality, const std::string& defaultText);
    std::string getVoiceIdFromType(VoiceType voice);

    // Test data structures - make sure these are properly declared with types
    std::map<std::pair<int, int>, RelationshipStatus> m_testRelationships;
    std::map<std::string, std::string> m_testQuestStates;
    std::map<std::string, int> m_testInventory;
    std::map<std::string, int> m_testPlayerStats;
    std::string m_testTimeOfDay = "Morning";
};