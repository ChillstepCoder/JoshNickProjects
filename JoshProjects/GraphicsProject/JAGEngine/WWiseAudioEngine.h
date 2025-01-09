// WWiseAudioEngine.h

#pragma once

#include <iostream>
#include <AK/SoundEngine/Common/AkMemoryMgr.h>
#include <AK/SoundEngine/Common/AkMemoryMgrModule.h>

#include <AK/SoundEngine/Common/IAkStreamMgr.h>
#include <AK/Tools/Common/AkPlatformFuncs.h>
#include <AkFilePackageLowLevelIO.h>
#include <AkFilePackageLowLevelIODeferred.h>

#include <AkFilePackage.h>
#include <AkFilePackageLUT.h>

#include <AK/SoundEngine/Common/Wwise_IDs.h>

#define BANKNAME_INIT L"Init.bnk"
#define BANKNAME_RAYCASTER L"RayCast.bnk"

#define WWISE_BANK_PATH AKTEXT("../WwiseProjects/RacingGame/GeneratedSoundBanks/Windows")

const AkGameObjectID GAME_OBJECT_ID_THEME = 100;

namespace JAGEngine {

  class WWiseAudioEngine
  {
  public:
    bool init();
  private:
    bool m_isInitialized = false;
  };

}
