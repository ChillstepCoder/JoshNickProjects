//DialogueAudioEngine.cpp

#include "DialogueAudioEngine.h"
#include <JAGEngine/WWiseAudioEngine.h>
#include <iostream>

DialogueAudioEngine::DialogueAudioEngine()
    : m_initialized(false),
    m_masterVolume(1.0f),
    m_voiceVolume(1.0f),
    m_effectsVolume(1.0f) {
}

DialogueAudioEngine::~DialogueAudioEngine() {
    cleanup();
}

bool DialogueAudioEngine::init(const std::string& projectName) {
    std::cout << "Initializing Dialogue Audio Engine...\n";

    // Create and initialize Wwise audio engine
    m_audioEngine = std::make_unique<JAGEngine::WWiseAudioEngine>();
    if (!m_audioEngine->initWithProject(projectName)) {
        std::cout << "Failed to initialize Wwise audio engine for " << projectName << "!\n";
        return false;
    }

    std::cout << "Wwise audio engine initialized successfully for " << projectName << "!\n";
    std::wcout << L"Using bank path: " << m_audioEngine->getBankPath() << std::endl;

    // Register game objects
    AKRESULT result = AK::SoundEngine::RegisterGameObj(m_voiceObjectId, "VoicePlayback");
    if (result != AK_Success) {
        std::cout << "Failed to register voice playback game object. Result: " << result << std::endl;
        return false;
    }

    result = AK::SoundEngine::RegisterGameObj(m_uiObjectId, "UIEvents");
    if (result != AK_Success) {
        std::cout << "Failed to register UI events game object. Result: " << result << std::endl;
        return false;
    }

    // Set initial volumes
    setMasterVolume(m_masterVolume);
    setVoiceVolume(m_voiceVolume);
    setEffectsVolume(m_effectsVolume);

    m_initialized = true;
    return true;
}

void DialogueAudioEngine::update() {
    if (m_audioEngine) {
        m_audioEngine->update();
    }
}

void DialogueAudioEngine::cleanup() {
    if (m_audioEngine) {
        // Unregister game objects
        AK::SoundEngine::UnregisterGameObj(m_voiceObjectId);
        AK::SoundEngine::UnregisterGameObj(m_uiObjectId);

        // Cleanup Wwise
        m_audioEngine->cleanup();
        m_audioEngine.reset();
    }
    m_initialized = false;
}

bool DialogueAudioEngine::playVoiceFile(const std::string& filePath) {
    if (!m_initialized) return false;

    // In a real implementation, you would use AK::SoundEngine::PostEvent with the appropriate
    // event ID for the voice file. For now, we'll just print a message.
    std::cout << "Playing voice file: " << filePath << std::endl;
    return true;
}

void DialogueAudioEngine::stopVoicePlayback() {
    if (!m_initialized) return;

    // Stop all events on the voice object
    AK::SoundEngine::StopAll(m_voiceObjectId);
}

void DialogueAudioEngine::playUISound(AkUniqueID soundId) {
    if (!m_initialized) return;

    AK::SoundEngine::PostEvent(soundId, m_uiObjectId);
}

void DialogueAudioEngine::playDialogueBegin() {
    if (!m_initialized) return;

    // Replace with your actual Wwise event ID
    AkUniqueID dialogueBeginEvent = 123456; // Example ID
    AK::SoundEngine::PostEvent(dialogueBeginEvent, m_uiObjectId);
}

void DialogueAudioEngine::playDialogueEnd() {
    if (!m_initialized) return;

    // Replace with your actual Wwise event ID
    AkUniqueID dialogueEndEvent = 654321; // Example ID
    AK::SoundEngine::PostEvent(dialogueEndEvent, m_uiObjectId);
}

void DialogueAudioEngine::setMasterVolume(float volume) {
    m_masterVolume = volume;
    if (m_initialized) {
        AK::SoundEngine::SetRTPCValue("Master_Volume", volume * 100.0f);
    }
}

void DialogueAudioEngine::setVoiceVolume(float volume) {
    m_voiceVolume = volume;
    if (m_initialized) {
        AK::SoundEngine::SetRTPCValue("Voice_Volume", volume * 100.0f);
    }
}

void DialogueAudioEngine::setEffectsVolume(float volume) {
    m_effectsVolume = volume;
    if (m_initialized) {
        AK::SoundEngine::SetRTPCValue("Effects_Volume", volume * 100.0f);
    }
}