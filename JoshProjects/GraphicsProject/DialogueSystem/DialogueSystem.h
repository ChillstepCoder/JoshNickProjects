//DialogueSystem.h

#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <functional>
#include <unordered_map>

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

private:
    static int s_nextId;

    int m_id;
    NodeType m_type;
    std::string m_text;
    std::vector<std::pair<std::shared_ptr<DialogueNode>, std::string>> m_children;
    std::shared_ptr<DialogueResponse> m_response;
    DialogueCondition m_condition;
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
    std::shared_ptr<DialogueNode> findNextNode(std::shared_ptr<DialogueNode> currentNode, int npcId, int playerId);

    bool evaluateCondition(const DialogueCondition& condition, int npcId, int playerId);
    bool checkQuestStatus(const std::string& questId, const std::string& status);
    bool checkInventoryItem(const std::string& itemId, int quantity);
    bool checkPlayerStat(const std::string& statName, int threshold);

    // Helper methods
    std::string buildAudioFilePath(ResponseType type, PersonalityType personality, VoiceType voice);
    AkUniqueID createDialogueEventID(ResponseType type, PersonalityType personality, VoiceType voice);

    std::shared_ptr<DialogueNode> findParentNode(int childId) const;
    bool removeChildFromParent(int childId);

    // Helper methods for DialogueManager class
    std::string getPersonalityName(PersonalityType type) const;
    std::string getResponseTypeName(ResponseType type) const;
    std::string getVoiceTypeName(VoiceType type) const;
    RelationshipStatus getRelationshipStatus(int npcId, int playerId);

    //test methods
    void setRelationshipForTesting(int npcId, int playerId, RelationshipStatus status);

private:
    JAGEngine::WWiseAudioEngine* m_audioEngine;
    std::string m_gptApiKey;
    std::string m_elevenLabsKey;
    std::string m_elevenLabsApiKey;
    std::unordered_map<ResponseType, std::shared_ptr<DialogueResponse>> m_responses;
    std::unordered_map<int, std::shared_ptr<DialogueNode>> m_nodes;

    // Helper methods
    std::string getPromptForPersonality(ResponseType type, PersonalityType personality, const std::string& defaultText);
    std::string getVoiceIdFromType(VoiceType voice);

    // Test data structures
    std::map<std::pair<int, int>, RelationshipStatus> m_testRelationships;
    std::map<std::string, std::string> m_testQuestStates;
    std::map<std::string, int> m_testInventory;
    std::map<std::string, int> m_testPlayerStats;
};