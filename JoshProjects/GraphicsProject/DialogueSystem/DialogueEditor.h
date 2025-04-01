//DialogueEditor.h
#pragma once

#include "DialogueSystem.h"
#include <vector>
#include <string>

// Forward declarations for ImGui - you don't need full ImGui.h here
struct ImVec2;
struct ImVec4;

// Dialogue editor (ImGui-based)
class DialogueEditor {
public:
    DialogueEditor(DialogueManager* manager);
    ~DialogueEditor();

    void initialize();
    void shutdown();
    void update();
    void render();

    // Editor state
    void setActiveResponse(ResponseType type);
    void showBatchGenerationWindow(bool show);

    // Test Methods
    void testDialogueNavigation();
    void createRelationshipTestTree();

private:
    DialogueManager* m_dialogueManager;
    ResponseType m_activeResponseType;
    PersonalityType m_selectedPersonality;
    VoiceType m_selectedVoice;
    bool m_showBatchGenerationWindow;
    bool m_showApiConfigWindow = false;
    int m_selectedNodeId = -1;

    std::vector<ResponseType> m_batchGenerationTypes;
    std::vector<PersonalityType> m_batchGenerationPersonalities;
    std::vector<VoiceType> m_batchGenerationVoices;


    // ImGui rendering helpers
    void renderResponseEditor();
    void renderDialogueTreeEditor();
    void renderAPISettingsWindow();
    void renderBatchGenerationWindow();
    void renderMenuBar();
    void renderNodeTree();
    void renderNodeProperties();

    // Helper methods
    void loadSavedDialogue();
    void saveDialogue();
    const char* getPersonalityName(PersonalityType type);
    const char* getResponseTypeName(ResponseType type);
    const char* getVoiceTypeName(VoiceType type);
};