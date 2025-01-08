#pragma once

#include <iostream>
#include "AK/SoundEngine/Common/AkMemoryMgr.h"
#include "AK/SoundEngine/Common/AkMemoryMgrModule.h"

namespace JAGEngine {

  class WWiseAudioEngine
  {
  public:
    bool init();
  private:
    bool m_isInitialized = false;
  };

}
