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

    m_isInitialized = true;
    return true;
  }
}
