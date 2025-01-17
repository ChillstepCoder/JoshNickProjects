// RacingAudioDefs.h

#pragma once
#include <AK/SoundEngine/Common/AkTypes.h>
#include "Wwise_IDs.h"  // Include Wwise generated header

// Game object IDs for various sound sources
namespace RacingAudio {
  // Game object IDs
  static const AkGameObjectID GAME_OBJECT_COUNTDOWN = 100;
  static const AkGameObjectID GAME_OBJECT_ENGINE = 101;
  static const AkGameObjectID GAME_OBJECT_TIRES = 102;
  static const AkGameObjectID GAME_OBJECT_IMPACTS = 103;
  static const AkGameObjectID GAME_OBJECT_MUSIC = 104;
  static const AkGameObjectID LISTENER_ID = 1;

  // RTPC IDs - direct use of the generated IDs
  static const AkRtpcID RTPC_ENGINE_IDLE_VOLUME = AK::GAME_PARAMETERS::ENGINE_IDLE_VOLUME;
  static const AkRtpcID RTPC_ENGINE_REV_VOLUME = AK::GAME_PARAMETERS::ENGINE_REV_VOLUME;
}

// Bank definitions
#define RACING_BANKNAME_MAIN L"Main.bnk"
#define RACING_BANK_PATH AKTEXT("../WwiseProjects/RacingGame/GeneratedSoundBanks/Windows")

