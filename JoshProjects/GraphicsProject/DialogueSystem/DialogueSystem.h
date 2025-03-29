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
        Statement,        // Simple NPC statement
        PlayerChoice,     // Player selects from options
        DynamicStatement  // Statement with variables
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

    // Response connections
    void setResponse(std::shared_ptr<DialogueResponse> response);
    std::shared_ptr<DialogueResponse> getResponse() const;

private:
    static int s_nextId;

    int m_id;
    NodeType m_type;
    std::string m_text;
    std::vector<std::pair<std::shared_ptr<DialogueNode>, std::string>> m_children;
    std::shared_ptr<DialogueResponse> m_response;
};

// Manages all dialogue responses and provides access to APIs
class DialogueManager {
public:
    DialogueManager();
    ~DialogueManager();

    // Initialization
    bool initialize(JAGEngine::WWiseAudioEngine* audioEngine);
    void shutdown();

    // Dialogue response management
    std::shared_ptr<DialogueResponse> createResponse(ResponseType type, const std::string& defaultText);
    std::shared_ptr<DialogueResponse> getResponse(ResponseType type);
    std::vector<std::shared_ptr<DialogueResponse>> getAllResponses() const;

    // Dialogue tree management
    std::shared_ptr<DialogueNode> createDialogueNode(DialogueNode::NodeType type, const std::string& text);
    std::shared_ptr<DialogueNode> getNodeById(int id);

    // API integrations (mocked for now)
    void setAPIKeys(const std::string& gptKey, const std::string& elevenLabsKey);
    std::string generateTextWithGPT(ResponseType type, PersonalityType personality, const std::string& defaultText);
    bool generateVoiceWithElevenLabs(const std::string& text, PersonalityType personality, VoiceType voice, const std::string& outputPath);

    // Mock batch operations
    void generateAllTextVariants();
    void generateAllVoiceVariants(VoiceType voice);

private:
    JAGEngine::WWiseAudioEngine* m_audioEngine;
    std::string m_gptApiKey;
    std::string m_elevenLabsApiKey;
    std::unordered_map<ResponseType, std::shared_ptr<DialogueResponse>> m_responses;
    std::unordered_map<int, std::shared_ptr<DialogueNode>> m_nodes;

    // Helper methods
    std::string getPromptForPersonality(ResponseType type, PersonalityType personality, const std::string& defaultText);
    std::string getVoiceIdFromType(VoiceType voice);
};