//DialogueAudioEngine.h

#pragma once

#include <memory>
#include <string>
#include <AK/SoundEngine/Common/AkTypes.h>

// Forward declarations
namespace JAGEngine {
    class WWiseAudioEngine;
}

enum class VoiceType;
enum class PersonalityType;

class DialogueAudioEngine {
public:
    DialogueAudioEngine();
    ~DialogueAudioEngine();

    bool init(const std::string& projectName = "DialogueDemo");
    void update();
    void cleanup();

    // Voice playback
    bool playVoiceFile(const std::string& filePath);
    void stopVoicePlayback();

    // Sound effects
    void playUISound(AkUniqueID soundId);
    void playDialogueBegin();
    void playDialogueEnd();

    // Getters
    JAGEngine::WWiseAudioEngine* getWWiseEngine() { return m_audioEngine.get(); }
    bool isInitialized() const { return m_initialized; }

    // Volume controls
    void setMasterVolume(float volume);
    void setVoiceVolume(float volume);
    void setEffectsVolume(float volume);
    float getMasterVolume() const { return m_masterVolume; }
    float getVoiceVolume() const { return m_voiceVolume; }
    float getEffectsVolume() const { return m_effectsVolume; }

private:
    std::unique_ptr<JAGEngine::WWiseAudioEngine> m_audioEngine;
    bool m_initialized = false;
    float m_masterVolume = 1.0f;
    float m_voiceVolume = 1.0f;
    float m_effectsVolume = 1.0f;
    AkGameObjectID m_voiceObjectId = 1;
    AkGameObjectID m_uiObjectId = 2;
};