//DialogueTreeView.cpp

#include "DialogueTreeView.h"
#include <ImGui/imgui.h>
#include <vector>
#include <string>
#include <functional>

// This function creates a sample dialogue tree similar to your diagram
void createSampleDialogueTree(DialogueManager* manager) {
    // Create responses if they don't exist
    if (!manager->getResponse(ResponseType::EnthusiasticAffirmative)) {
        manager->createResponse(ResponseType::EnthusiasticAffirmative, "Absolutely!");
    }

    if (!manager->getResponse(ResponseType::IndifferentAffirmative)) {
        manager->createResponse(ResponseType::IndifferentAffirmative, "Yeah, sure.");
    }

    if (!manager->getResponse(ResponseType::EnthusiasticNegative)) {
        manager->createResponse(ResponseType::EnthusiasticNegative, "No way!");
    }

    if (!manager->getResponse(ResponseType::MarkOnMap)) {
        manager->createResponse(ResponseType::MarkOnMap, "Let me mark that on your map.");
    }

    if (!manager->getResponse(ResponseType::UntrustworthyResponse)) {
        manager->createResponse(ResponseType::UntrustworthyResponse, "I definitely won't tell anyone...");
    }

    if (!manager->getResponse(ResponseType::Confused)) {
        manager->createResponse(ResponseType::Confused, "Wait, what?");
    }

    // Create the dialogue tree nodes
    auto rootNode = manager->createDialogueNode(
        DialogueNode::NodeType::NPCStatement,
        "Do you know where..."
    );

    auto johnNode = manager->createDialogueNode(
        DialogueNode::NodeType::NPCStatement,
        "John is?"
    );

    auto yesNode = manager->createDialogueNode(
        DialogueNode::NodeType::PlayerChoice,
        "Yes"
    );
    yesNode->setResponse(manager->getResponse(ResponseType::EnthusiasticAffirmative));

    auto noNode = manager->createDialogueNode(
        DialogueNode::NodeType::PlayerChoice,
        "No"
    );
    noNode->setResponse(manager->getResponse(ResponseType::EnthusiasticNegative));

    auto likeNode = manager->createDialogueNode(
        DialogueNode::NodeType::NPCStatement,
        "Do you like me?"
    );

    auto likeYesNode = manager->createDialogueNode(
        DialogueNode::NodeType::PlayerChoice,
        "Yes"
    );
    likeYesNode->setResponse(manager->getResponse(ResponseType::EnthusiasticAffirmative));

    auto likeNoNode = manager->createDialogueNode(
        DialogueNode::NodeType::PlayerChoice,
        "No"
    );
    likeNoNode->setResponse(manager->getResponse(ResponseType::IndifferentAffirmative));

    auto mapNode = manager->createDialogueNode(
        DialogueNode::NodeType::NPCStatement,
        "Let me mark it on your map."
    );
    mapNode->setResponse(manager->getResponse(ResponseType::MarkOnMap));

    auto whyNode = manager->createDialogueNode(
        DialogueNode::NodeType::NPCStatement,
        "Why?"
    );

    auto dontTrustNode = manager->createDialogueNode(
        DialogueNode::NodeType::PlayerChoice,
        "I don't trust you"
    );
    dontTrustNode->setResponse(manager->getResponse(ResponseType::UntrustworthyResponse));

    auto dontKnowNode = manager->createDialogueNode(
        DialogueNode::NodeType::PlayerChoice,
        "I don't know"
    );
    dontKnowNode->setResponse(manager->getResponse(ResponseType::Confused));

    // Connect the nodes
    rootNode->addChildNode(johnNode);
    johnNode->addChildNode(yesNode);
    johnNode->addChildNode(noNode);
    yesNode->addChildNode(likeNode);
    likeNode->addChildNode(likeYesNode);
    likeNode->addChildNode(likeNoNode);
    likeYesNode->addChildNode(mapNode);
    likeNoNode->addChildNode(mapNode);
    noNode->addChildNode(whyNode);
    whyNode->addChildNode(dontTrustNode);
    whyNode->addChildNode(dontKnowNode);
}

// Render the dialogue tree with ImGui
void renderDialogueTreeView(DialogueManager* manager, std::shared_ptr<DialogueNode> rootNode) {
    if (!rootNode) return;

    ImGui::SetNextWindowPos(ImVec2(500, 50), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_FirstUseEver);

    if (ImGui::Begin("Dialogue Tree Viewer")) {
        // Display function for recursive tree rendering
        std::function<void(std::shared_ptr<DialogueNode>, int)> displayNode;

        displayNode = [&displayNode, manager](std::shared_ptr<DialogueNode> node, int depth) {
            if (!node) return;

            // Create indentation based on depth
            ImGui::Indent(depth * 20.0f);

            // Process text using the dialogue manager so placeholders (like [NPC1]) are replaced
            std::string nodeText = manager->processTextWithTreeParameters(node->getId(), node->getText());
            std::string typeText;

            switch (node->getType()) {
            case DialogueNode::NodeType::NPCStatement:
                typeText = "[Statement]";
                break;
            case DialogueNode::NodeType::PlayerChoice:
                typeText = "[Player Choice]";
                break;
            case DialogueNode::NodeType::DynamicStatement:
                typeText = "[Dynamic]";
                break;
            }

            // Get response text if available
            std::string responseText;
            auto response = node->getResponse();
            if (response) {
                responseText = " -> \"" + response->getTextForPersonality(PersonalityType::Bubbly) + "\"";
            }

            ImGui::Text("%s %s%s", nodeText.c_str(), typeText.c_str(), responseText.c_str());

            // Display children nodes
            for (const auto& childPair : node->getChildren()) {
                std::string condition = childPair.second;
                if (!condition.empty()) {
                    ImGui::Indent(depth * 20.0f + 10.0f);
                    ImGui::Text("(Condition: %s)", condition.c_str());
                    ImGui::Unindent(depth * 20.0f + 10.0f);
                }
                displayNode(childPair.first, depth + 1);
            }

            ImGui::Unindent(depth * 20.0f);
        };


        // Start rendering from the root
        ImGui::Text("Dialogue Tree Structure:");
        ImGui::Separator();

        displayNode(rootNode, 0);
    }
    ImGui::End();
}

// Render a dialogue player interface
void renderDialoguePlayer(DialogueManager* manager, std::shared_ptr<DialogueNode> currentNode) {
    if (!currentNode) return;

    ImGui::SetNextWindowPos(ImVec2(500, 500), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(500, 200), ImGuiCond_FirstUseEver);

    if (ImGui::Begin("Dialogue Player")) {
        // Display current node text
        ImGui::TextWrapped("NPC: %s", currentNode->getText().c_str());

        // If node has a response, show it
        auto response = currentNode->getResponse();
        if (response) {
            // Get response for bubbly personality as an example
            std::string responseText = response->getTextForPersonality(PersonalityType::Bubbly);
            ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "NPC Response: \"%s\"", responseText.c_str());
        }

        ImGui::Separator();

        // Show children nodes as choice buttons
        auto children = currentNode->getChildren();
        if (!children.empty()) {
            ImGui::Text("Your choices:");

            for (const auto& childPair : children) {
                if (ImGui::Button(childPair.first->getText().c_str(), ImVec2(ImGui::GetContentRegionAvail().x, 0))) {
                    // In a real implementation, this would update the current node
                    // But for simplicity, we'll just display the selection
                    ImGui::OpenPopup("Selection");
                }
            }

            if (ImGui::BeginPopup("Selection")) {
                ImGui::Text("You selected this option!");
                ImGui::EndPopup();
            }
        }
    }
    ImGui::End();
}