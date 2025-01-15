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

    std::cout << "Verifying sound setup..." << std::endl;

    const char* eventNames[] = { "Play_Countdown_SFX_1", "Play_Countdown_SFX_2" };
    AKRESULT prepareResult = AK::SoundEngine::PrepareEvent(
      AK::SoundEngine::Preparation_Load,
      eventNames,
      2
    );

    if (prepareResult != AK_Success) {
        std::cout << "Failed to prepare Events. Error code: " << prepareResult << std::endl;
    }
    else {
      std::cout << "Events prepared successfully" << std::endl;
    }

    const AkGameObjectID TEST_OBJECT_ID = 999;

    const char* defaultShareSet = nullptr;
    AkUInt32 deviceID = 0;
    AkOutputDeviceID outputID = AK::SoundEngine::GetOutputID(defaultShareSet, deviceID);
    std::cout << "Output device ID: " << outputID << std::endl;

    AKRESULT outputResult = AK::SoundEngine::SetDefaultListeners(&TEST_OBJECT_ID, 1);
    std::cout << "Set default listener result: " << outputResult << std::endl;

    AKRESULT regResult = AK::SoundEngine::RegisterGameObj(TEST_OBJECT_ID, "TestObject");
    if (regResult != AK_Success) {
      std::cout << "Failed to register test object. Error: " << regResult << std::endl;
    }
    else {
      std::cout << "Test object registered successfuly." << std::endl;
    }

    // Explicit volume levels

    AK::SoundEngine::SetRTPCValue(AKTEXT("Master_Volume"), 100.0f);
    AK::SoundEngine::SetRTPCValue(AKTEXT("Effects_Volume"), 100.0f);

    AkPlayingID testID = AK::SoundEngine::PostEvent(
      AKTEXT("Play_Countdown_SFX_1"),
      TEST_OBJECT_ID,
      AK_EndOfEvent,
      [](AkCallbackType type, AkCallbackInfo* info) {
        std::cout << "Sound callback received, type: " << type << std::endl;
      },
      nullptr
    );

    std::cout << "Test sound play attempt - ID: " << testID << std::endl;

    m_isInitialized = true;
    return true;
  }

  void WWiseAudioEngine::update() {
    if (m_isInitialized) {
      static uint32_t frameCount = 0;
      frameCount++;

      AKRESULT result = AK::SoundEngine::RenderAudio();
      if (result != AK_Success) {
        std::cout << "Error rendering audio. Result: " << result << std::endl;
      }

      // Add detailed logging every few seconds
      if (frameCount % 300 == 0) {  // Every ~5 seconds at 60fps
        std::cout << "Audio frame " << frameCount
          << " - Memory status: " << (AK::MemoryMgr::IsInitialized() ? "OK" : "Failed")
          << " - Sound Engine: " << (AK::SoundEngine::IsInitialized() ? "OK" : "Failed")
          << std::endl;
      }
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
