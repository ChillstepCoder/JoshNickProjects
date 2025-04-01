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

        // Create a sample dialogue tree for testing
        testDialogueTreeEditor(m_dialogueManager.get());
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

    void testDialogueTreeEditor(DialogueManager* manager) {
        // Create a sample dialogue tree for testing

        // 1. Create root node - Player question
        auto rootNode = manager->createDialogueNode(
            DialogueNode::NodeType::PlayerChoice,
            "Do you know where..."
        );

        // 2. Create child nodes
        auto johnNode = manager->createDialogueNode(
            DialogueNode::NodeType::PlayerChoice,
            "John is?"
        );

        auto yesNode = manager->createDialogueNode(
            DialogueNode::NodeType::PlayerChoice,
            "Yes"
        );

        auto noNode = manager->createDialogueNode(
            DialogueNode::NodeType::PlayerChoice,
            "No"
        );

        // 3. Connect nodes
        rootNode->addChildNode(johnNode);
        johnNode->addChildNode(yesNode);
        johnNode->addChildNode(noNode);

        // 4. Add responses to nodes
        auto enthusiasticResponse = manager->getResponse(ResponseType::EnthusiasticAffirmative);
        auto negativeResponse = manager->getResponse(ResponseType::IndifferentNegative);

        yesNode->setResponse(enthusiasticResponse);
        noNode->setResponse(negativeResponse);

        // Create a deeper branch for testing
        auto likeNode = manager->createDialogueNode(
            DialogueNode::NodeType::NPCStatement,
            "Do you like me?"
        );

        auto likeYesNode = manager->createDialogueNode(
            DialogueNode::NodeType::PlayerChoice,
            "Yes"
        );

        auto likeNoNode = manager->createDialogueNode(
            DialogueNode::NodeType::PlayerChoice,
            "No"
        );

        yesNode->addChildNode(likeNode);
        likeNode->addChildNode(likeYesNode);
        likeNode->addChildNode(likeNoNode);

        // Set responses for these nodes
        likeYesNode->setResponse(manager->getResponse(ResponseType::EnthusiasticAffirmative));
        likeNoNode->setResponse(manager->getResponse(ResponseType::IndifferentNegative));

        // Add a final node
        auto mapNode = manager->createDialogueNode(
            DialogueNode::NodeType::NPCStatement,
            "Let me mark it on your map."
        );

        likeYesNode->addChildNode(mapNode);
        likeNoNode->addChildNode(mapNode);

        mapNode->setResponse(manager->getResponse(ResponseType::MarkOnMap));
    }

    void createRelationshipTestTree(DialogueManager* manager) {
        // Create responses if they don't exist
        if (!manager->getResponse(ResponseType::EnthusiasticAffirmative)) {
            manager->createResponse(ResponseType::EnthusiasticAffirmative, "Absolutely!");
        }

        if (!manager->getResponse(ResponseType::IndifferentAffirmative)) {
            manager->createResponse(ResponseType::IndifferentAffirmative, "Yeah, sure.");
        }

        if (!manager->getResponse(ResponseType::MarkOnMap)) {
            manager->createResponse(ResponseType::MarkOnMap, "Let me mark that on your map.");
        }

        if (!manager->getResponse(ResponseType::Farewell)) {
            manager->createResponse(ResponseType::Farewell, "See you around!");
        }

        // Create root node - starting dialogue
        auto rootNode = manager->createDialogueNode(
            DialogueNode::NodeType::PlayerChoice,
            "Do you know where John is?"
        );

        // Create a relationship check node (using ConditionCheck)
        auto relationshipNode = manager->createDialogueNode(
            DialogueNode::NodeType::ConditionCheck,
            "Check if NPC likes player"
        );

        // Set the relationship condition to check if NPC is friendly or better
        relationshipNode->setCondition(
            DialogueCondition::CreateRelationshipCondition(RelationshipStatus::Friendly)
        );

        // Create response nodes based on relationship
        // If relationship is Friendly or better
        auto friendlyResponse = manager->createDialogueNode(
            DialogueNode::NodeType::NPCStatement,
            "Friendly response"
        );
        friendlyResponse->setResponse(manager->getResponse(ResponseType::EnthusiasticAffirmative));

        // If relationship is Neutral or worse
        auto neutralResponse = manager->createDialogueNode(
            DialogueNode::NodeType::NPCStatement,
            "Neutral response"
        );
        neutralResponse->setResponse(manager->getResponse(ResponseType::IndifferentAffirmative));

        // Create the map marking node - both paths lead here
        auto mapNode = manager->createDialogueNode(
            DialogueNode::NodeType::NPCStatement,
            "Mark location on map"
        );
        mapNode->setResponse(manager->getResponse(ResponseType::MarkOnMap));

        // Create player choices to demonstrate more branching
        auto choicesNode = manager->createDialogueNode(
            DialogueNode::NodeType::NPCStatement,
            "Can I ask you something else?"
        );

        auto yesChoice = manager->createDialogueNode(
            DialogueNode::NodeType::PlayerChoice,
            "Yes"
        );

        auto noChoice = manager->createDialogueNode(
            DialogueNode::NodeType::PlayerChoice,
            "No"
        );

        // Additional response for more dialogue
        auto moreDialogue = manager->createDialogueNode(
            DialogueNode::NodeType::NPCStatement,
            "What else do you want to know?"
        );

        auto endDialogue = manager->createDialogueNode(
            DialogueNode::NodeType::NPCStatement,
            "See you around then."
        );
        endDialogue->setResponse(manager->getResponse(ResponseType::Farewell));

        // Connect all the nodes together

        // Initial query connects to relationship check
        rootNode->addChildNode(relationshipNode);

        // Relationship check branches based on relationship status
        relationshipNode->addChildNode(friendlyResponse, "Friendly");
        relationshipNode->addChildNode(neutralResponse, "Neutral/Unfriendly");

        // Both relationship responses lead to map marker
        friendlyResponse->addChildNode(mapNode);
        neutralResponse->addChildNode(mapNode);

        // After map marker, go to follow-up question
        mapNode->addChildNode(choicesNode);

        // Follow-up question offers player choices
        choicesNode->addChildNode(yesChoice);
        choicesNode->addChildNode(noChoice);

        // Player choices lead to different outcomes
        yesChoice->addChildNode(moreDialogue);
        noChoice->addChildNode(endDialogue);

        // For testing, set up a relationship
        manager->setRelationshipForTesting(1, 1, RelationshipStatus::Friendly); // NPC 1 likes player 1
        manager->setRelationshipForTesting(2, 1, RelationshipStatus::Unfriendly); // NPC 2 dislikes player 1
    }

    void testDialogueNavigation() {
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

    std::unique_ptr<DialogueAudioEngine> m_audioEngine;
    std::unique_ptr<DialogueManager> m_dialogueManager;
    std::unique_ptr<DialogueEditor> m_dialogueEditor;
};

