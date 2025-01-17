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

    // Set up bank path and language
    m_lowLevelIO.SetBasePath(WWISE_BANK_PATH);
    AK::StreamMgr::SetCurrentLanguage(AKTEXT("English(US)"));

    // Load banks
    AkBankID bankID;
    AKRESULT result;

    // Load Init bank first
    result = AK::SoundEngine::LoadBank(BANKNAME_INIT, bankID);
    if (result != AK_Success) {
      std::cout << "Failed to load Init bank. Error code: " << result << std::endl;
      return false;
    }
    std::cout << "Init Bank loaded successfully." << std::endl;

    // Load Main bank
    result = AK::SoundEngine::LoadBank(BANKNAME_MAIN, bankID);
    if (result != AK_Success) {
      std::cout << "Failed to load Main bank. Error code: " << result << std::endl;
      return false;
    }
    std::cout << "Main Bank loaded successfully." << std::endl;

    // Initialize audio output
    const char* defaultShareSet = nullptr;
    AkUInt32 deviceID = 0;
    AkOutputDeviceID outputID = AK::SoundEngine::GetOutputID(defaultShareSet, deviceID);
    std::cout << "Output device ID: " << outputID << std::endl;

    // Set up default listener
    const AkGameObjectID DEFAULT_LISTENER_ID = 0;  // Using 0 as default listener
    AKRESULT listenerResult = AK::SoundEngine::RegisterGameObj(DEFAULT_LISTENER_ID, "DefaultListener");
    if (listenerResult != AK_Success) {
      std::cout << "Failed to register default listener" << std::endl;
      return false;
    }

    AKRESULT outputResult = AK::SoundEngine::SetDefaultListeners(&DEFAULT_LISTENER_ID, 1);
    if (outputResult != AK_Success) {
      std::cout << "Failed to set default listener" << std::endl;
      return false;
    }

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
