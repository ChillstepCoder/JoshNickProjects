// RacingAudioDefs.h

#pragma once
#include <AK/SoundEngine/Common/AkTypes.h>

// Game object IDs for various sound sources
namespace RacingAudio {
  static const AkGameObjectID GAME_OBJECT_COUNTDOWN = 100;
  static const AkGameObjectID GAME_OBJECT_ENGINE = 101;
  static const AkGameObjectID GAME_OBJECT_TIRES = 102;
  static const AkGameObjectID GAME_OBJECT_IMPACTS = 103;
  static const AkGameObjectID GAME_OBJECT_MUSIC = 104;
}

// Bank definitions
#define RACING_BANKNAME_MAIN L"Main.bnk"
#define RACING_BANK_PATH AKTEXT("../WwiseProjects/RacingGame/GeneratedSoundBanks/Windows")

