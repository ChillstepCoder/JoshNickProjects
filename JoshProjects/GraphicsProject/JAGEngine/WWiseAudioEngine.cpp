// WWiseAudioEngine.cpp

#include "WWiseAudioEngine.h"
#include "JAGErrors.h"

namespace JAGEngine {

  bool WWiseAudioEngine::init() {
    if (m_isInitialized) {
      fatalError("AK: Tried to initialize Audio Engine twice!");
    }

    AkMemSettings memSettings;
    AK::MemoryMgr::GetDefaultSettings(memSettings);

    //error checking
    if (AK::MemoryMgr::Init(&memSettings) == AK_Success) {
      std::cout << "AK: Memory Manager Initialized." << std::endl;

    }
    else {
      std::cout << "AK: Memory Manager Failed to Initialize." << std::endl;
      return false;
    }

    AkStreamMgrSettings stmSettings;
    AK::StreamMgr::GetDefaultSettings(stmSettings);

    //error checking
    if (!AK::StreamMgr::Create(stmSettings)) {
      std::cout << "AK: Stream Manager Failed to Create." << std::endl;
    }
    else {
      std::cout << "AK: Stream Manager Create." << std::endl;
      return false;
    }

    AkDeviceSettings deviceSettings;
    AK::StreamMgr::GetDefaultDeviceSettings(deviceSettings);

    CAkFilePackageLowLevelIODeferred g_lowLevelIO;

    if (g_lowLevelIO.Init(deviceSettings) == AK_Success) {
      std::cout << "AK: Created the streaming device and Low-Level I/O system." << std::endl;
    }
    else {
      std::cout << "AK: Could not create the streaming device and Low-Level I/O system." << std::endl;
    }

    AkInitSettings initSettings;
    AkPlatformInitSettings platformInitSettings;

    AK::SoundEngine::GetDefaultInitSettings(initSettings);
    AK::SoundEngine::GetDefaultPlatformInitSettings(platformInitSettings);

    if (AK::SoundEngine::Init(&initSettings, &platformInitSettings) == AK_Success) {
      std::cout << "AK: Sound Engine Initialized!" << std::endl;
    }
    else {
      std::cout << "AK: Could not initialize the Sound Engine." << std::endl;
    }

    g_lowLevelIO.SetBasePath(WWISE_BANK_PATH);
    AK::StreamMgr::SetCurrentLanguage(AKTEXT("English(US)"));

    AkBankID bankID;

    if (AK::SoundEngine::LoadBank(BANKNAME_INIT, bankID) != AK_Success) {
      std::cout << "AK: Could not load Init Bank." << std::endl;
      return false;
    }
    else {
      std::cout << "AK: init Bank Loaded!" << std::endl;
    }

    if (AK::SoundEngine::LoadBank(BANKNAME_RAYCASTER, bankID) != AK_Success) {
      std::cout << "AK: Could not load Raycaster Bank." << std::endl;
      return false;
    }
    else {
      std::cout << "AK: Raycaster Bank Loaded!" << std::endl;
    }


    m_isInitialized = true;
    return true;
  }
}
