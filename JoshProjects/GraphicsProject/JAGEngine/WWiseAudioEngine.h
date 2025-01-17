// WWiseAudioEngine.h

#pragma once

#include <windows.h>
#include <iostream>

#include <AK/SoundEngine/Common/AkMemoryMgr.h>
#include <AK/SoundEngine/Common/AkMemoryMgrModule.h>

#include <AK/SoundEngine/Common/IAkStreamMgr.h>
#include <AK/Tools/Common/AkPlatformFuncs.h>
#include <AkFilePackageLowLevelIO.h>
#include <AkFilePackageLowLevelIODeferred.h>

#include <AkFilePackage.h>
#include <AkFilePackageLUT.h>

#include <Wwise_IDs.h>

#define WWISE_BANK_PATH AKTEXT("../WwiseProjects/RacingGame/GeneratedSoundBanks/Windows/")
#define BANKNAME_INIT L"Init.bnk"
#define BANKNAME_MAIN L"Main.bnk"

namespace JAGEngine {

  class WWiseAudioEngine
  {
  public:
    bool init();
    void update();
    void cleanup();
    bool isInitialized() const { return m_isInitialized; }

  private:
    bool m_isInitialized = false;
    CAkFilePackageLowLevelIODeferred m_lowLevelIO;
    uint32_t m_frameCounter = 0;
  };

}
