//DialogueTreeView.h

#pragma once

#include "DialogueSystem.h"
#include <memory>

// Forward declarations for ImGui - you don't need full ImGui.h here
struct ImVec2;
struct ImVec4;

// Creates a sample dialogue tree for demonstration
void createSampleDialogueTree(DialogueManager* manager);

// Renders a structured view of a dialogue tree with ImGui
void renderDialogueTreeView(DialogueManager* manager, std::shared_ptr<DialogueNode> rootNode);

// Renders an interactive dialogue player interface for testing conversations
void renderDialoguePlayer(DialogueManager* manager, std::shared_ptr<DialogueNode> currentNode);