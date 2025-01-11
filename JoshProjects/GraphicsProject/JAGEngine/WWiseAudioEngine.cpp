// WWiseAudioEngine.cpp

#include "WWiseAudioEngine.h"
#include "JAGErrors.h"

namespace JAGEngine {

  bool WWiseAudioEngine::init() {
    if (m_isInitialized) {
      fatalError("AK: Tried to initialize Audio Engine twice!");
      return false;
    }

    // Initialize Memory Manager
    AkMemSettings memSettings;
    AK::MemoryMgr::GetDefaultSettings(memSettings);

    if (AK::MemoryMgr::Init(&memSettings) != AK_Success) {
      std::cout << "AK: Memory Manager Failed to Initialize." << std::endl;
      return false;
    }
    std::cout << "AK: Memory Manager Initialized." << std::endl;

    // Initialize Stream Manager
    AkStreamMgrSettings stmSettings;
    AK::StreamMgr::GetDefaultSettings(stmSettings);

    if (!AK::StreamMgr::Create(stmSettings)) {
      std::cout << "AK: Stream Manager Failed to Create." << std::endl;
      return false;
    }
    std::cout << "AK: Stream Manager Created Successfully." << std::endl;

    // Initialize Streaming Device
    AkDeviceSettings deviceSettings;
    AK::StreamMgr::GetDefaultDeviceSettings(deviceSettings);

    if (m_lowLevelIO.Init(deviceSettings) != AK_Success) {
      std::cout << "AK: Could not create the streaming device and Low-Level I/O system." << std::endl;
      return false;
    }
    std::cout << "AK: Created the streaming device and Low-Level I/O system." << std::endl;

    // Initialize Sound Engine
    AkInitSettings initSettings;
    AkPlatformInitSettings platformInitSettings;
    AK::SoundEngine::GetDefaultInitSettings(initSettings);
    AK::SoundEngine::GetDefaultPlatformInitSettings(platformInitSettings);

    if (AK::SoundEngine::Init(&initSettings, &platformInitSettings) != AK_Success) {
      std::cout << "AK: Could not initialize the Sound Engine." << std::endl;
      return false;
    }
    std::cout << "AK: Sound Engine Initialized!" << std::endl;

    // Success!
    m_isInitialized = true;
    return true;
  }

  void WWiseAudioEngine::update() {
    if (m_isInitialized) {
      AK::SoundEngine::RenderAudio();
    }
  }

  void WWiseAudioEngine::cleanup() {
    if (m_isInitialized) {
      AK::SoundEngine::Term();

      m_lowLevelIO.Term();
      if (AK::IAkStreamMgr::Get())
        AK::IAkStreamMgr::Get()->Destroy();

      AK::MemoryMgr::Term();

      m_isInitialized = false;
    }
  }
}
