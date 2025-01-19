
// AudioEngine.cpp

#include "AudioEngine.h"
#include "Car.h"
#include <JAGEngine/WWiseAudioEngine.h>
#include <iostream>

AudioEngine::AudioEngine() :
  m_isBoostPlaying(false),
  m_isEnginePlaying(false),
  m_currentRPM(0.0f),
  m_currentSpeed(0.0f),
  m_currentSurfaceType(0) {
}

AudioEngine::~AudioEngine() {
  cleanup();
}

bool AudioEngine::init() {
  std::cout << "Initializing Racing Audio Engine...\n";

  m_audioEngine = std::make_unique<JAGEngine::WWiseAudioEngine>();
  if (!m_audioEngine->init()) {
    std::cout << "Failed to initialize base audio engine!\n";
    return false;
  }

  // Initialize racing-specific audio settings
  // 
  // Set up RTPC parameters
  std::cout << "Registering audio game objects...\n";
  AkGameObjectID countdownID = RacingAudio::GAME_OBJECT_COUNTDOWN;
  AKRESULT result = AK::SoundEngine::RegisterGameObj(countdownID, "Countdown");
  if (result != AK_Success) {
    std::cout << "Failed to register countdown game object. Result: " << result << std::endl;
    return false;
  }
  std::cout << "Game objects registered successfully\n";

  // Set initial volumes
  m_masterVolume = 1.0f;
  m_effectsVolume = 1.0f;
  m_musicVolume = 1.0f;

  // Apply volumes to WWise
  AK::SoundEngine::SetRTPCValue("Master_Volume", m_masterVolume * 100.0f, RacingAudio::GAME_OBJECT_COUNTDOWN);
  AK::SoundEngine::SetRTPCValue("Effects_Volume", m_effectsVolume * 100.0f, RacingAudio::GAME_OBJECT_COUNTDOWN);
  AK::SoundEngine::SetRTPCValue("Music_Volume", m_musicVolume * 100.0f, RacingAudio::GAME_OBJECT_COUNTDOWN);

  // Print current volume settings
  std::cout << "Volume settings - Master: " << m_masterVolume
    << " Effects: " << m_effectsVolume
    << " Music: " << m_musicVolume << std::endl;

  // Apply volumes to WWise
  AK::SoundEngine::SetRTPCValue("Master_Volume", m_masterVolume * 100.0f);
  AK::SoundEngine::SetRTPCValue("Effects_Volume", m_effectsVolume * 100.0f);
  AK::SoundEngine::SetRTPCValue("Music_Volume", m_musicVolume * 100.0f);

  // Set up default listener
  const AkGameObjectID DEFAULT_LISTENER_ID = RacingAudio::LISTENER_ID;
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

  std::cout << "Default listener registered and set" << std::endl;

  std::cout << "Racing Audio Engine initialized successfully!\n";
  return true;
}

void AudioEngine::update() {
  if (m_audioEngine) {
    m_audioEngine->update();
  }
}

void AudioEngine::cleanup() {
  if (m_audioEngine) {
    // Stop all sounds
    m_isBoostPlaying = false;
    m_isEnginePlaying = false;
  }
}

void AudioEngine::playEngineSound(float rpm, float load) {
  
}

void AudioEngine::playTireSkidSound(float slipAmount) {

}

void AudioEngine::playCollisionSound(float impactForce) {

}

void AudioEngine::playBoostSound() {

}

void AudioEngine::stopBoostSound() {

}

void AudioEngine::playCountdownBeep() {
  if (m_audioEngine && m_audioEngine->isInitialized()) {
    AkPlayingID playingID = AK::SoundEngine::PostEvent(
      AK::EVENTS::PLAY_COUNTDOWN_SFX_1,
      RacingAudio::GAME_OBJECT_COUNTDOWN
    );
    if (playingID == AK_INVALID_PLAYING_ID) {
      std::cout << "Failed to play countdown beep" << std::endl;
    }
  }
}

void AudioEngine::playCountdownStart() {
  if (m_audioEngine && m_audioEngine->isInitialized()) {
    AkPlayingID playingID = AK::SoundEngine::PostEvent(
      AK::EVENTS::PLAY_COUNTDOWN_SFX_2,
      RacingAudio::GAME_OBJECT_COUNTDOWN
    );
    if (playingID == AK_INVALID_PLAYING_ID) {
      std::cout << "Failed to play countdown start" << std::endl;
    }
  }
}

void AudioEngine::playLapCompletedSound() {

}

void AudioEngine::playRaceStartSound() {

}

void AudioEngine::playRaceFinishSound() {

}

void AudioEngine::playCheckpointSound() {

}


void AudioEngine::setEngineRPM(float rpm) {
  m_currentRPM = rpm;
}

void AudioEngine::setCarSpeed(float speed) {
  m_currentSpeed = speed;
}

void AudioEngine::setTireSurfaceType(int surfaceType) {
  m_currentSurfaceType = surfaceType;
}

void AudioEngine::setDefaultListener(const Vec2& position, float rotation) {
  AkListenerPosition listenerPos;

  // Position with scale
  float positionScale = 1.0f;

  AkVector position3D;
  position3D.X = position.x * positionScale;
  position3D.Y = position.y * positionScale;
  position3D.Z = 0.0f;

  // Forward vector from rotation
  AkVector orientFront;
  orientFront.X = cos(rotation);
  orientFront.Y = sin(rotation);
  orientFront.Z = 0.0f;

  // Up vector
  AkVector orientTop;
  orientTop.X = 0.0f;
  orientTop.Y = 0.0f;
  orientTop.Z = 1.0f;

  listenerPos.Set(position3D, orientFront, orientTop);
  if (DEBUG_OUTPUT) {
    std::cout << "Setting listener position: (" << position3D.X << ", " << position3D.Y
      << ") rotation: " << rotation << std::endl;
  }
  // Set default listener
  AKRESULT result = AK::SoundEngine::SetDefaultListeners(&RacingAudio::LISTENER_ID, 1);
  if (DEBUG_OUTPUT) {
    if (result != AK_Success) {
      std::cout << "Failed to set default listener" << std::endl;
    }
  }

  result = AK::SoundEngine::SetPosition(RacingAudio::LISTENER_ID, listenerPos);
  if (DEBUG_OUTPUT) {
    if (result != AK_Success) {
      std::cout << "Failed to set listener position" << std::endl;
    }
  }
}

void AudioEngine::setObjectPosition(AkGameObjectID id, const Vec2& position) const {
  if (!m_audioEngine) return;

  float positionScale = 1.0f;

  AkSoundPosition soundPos;
  AkVector position3D;
  position3D.X = position.x * positionScale;
  position3D.Y = position.y * positionScale;
  position3D.Z = 0.0f;

  // Create default orientation
  AkVector orientFront;
  orientFront.X = 1.0f;
  orientFront.Y = 0.0f;
  orientFront.Z = 0.0f;

  AkVector orientTop;
  orientTop.X = 0.0f;
  orientTop.Y = 0.0f;
  orientTop.Z = 1.0f;

  soundPos.Set(position3D, orientFront, orientTop);
  AK::SoundEngine::SetPosition(id, soundPos);
}

void AudioEngine::setMasterVolume(float volume) {
  m_masterVolume = volume;
}

void AudioEngine::setEffectsVolume(float volume) {
  m_effectsVolume = volume;
}

void AudioEngine::setMusicVolume(float volume) {
  m_musicVolume = volume;
}

void AudioEngine::initializeCarAudio(Car* car) {
  if (DEBUG_OUTPUT) {
    if (!m_audioEngine) {
      std::cout << "ERROR: No audio engine when initializing car audio!" << std::endl;
      return;
    }
    if (!car) {
      std::cout << "ERROR: Null car when initializing car audio!" << std::endl;
      return;
    }
  }

  // Create unique audio ID for this car
  AkGameObjectID audioId = car->getAudioId();
  if (DEBUG_OUTPUT) {
    std::cout << "Initializing audio for car " << (unsigned)audioId << std::endl;
  }

  // Register the car's game object
  AKRESULT result = AK::SoundEngine::RegisterGameObj(audioId, "CarEngine");
  if (DEBUG_OUTPUT) {
    std::cout << "RegisterGameObj result: " << result << std::endl;
    if (result != AK_Success) {
      std::cout << "Failed to register car audio object. Result: " << result << std::endl;
      return;
    }
    std::cout << "Successfully registered car audio object" << std::endl;
  }

  // Start playing the idle sound (loops infinitely)
  AkPlayingID idleId = AK::SoundEngine::PostEvent(
    AK::EVENTS::PLAY_ENGINE_IDLE_SFX_1,
    audioId
  );
  if (DEBUG_OUTPUT) {
    std::cout << "Posted idle sound event, ID: " << idleId << " Event ID: " << AK::EVENTS::PLAY_ENGINE_IDLE_SFX_1 << std::endl;
    if (idleId == AK_INVALID_PLAYING_ID) {
      std::cout << "ERROR: Failed to play idle sound!" << std::endl;
    }
  }

  // Start playing the rev sound (at 0 volume initially)
  AkPlayingID revId = AK::SoundEngine::PostEvent(
    AK::EVENTS::PLAY_ENGINE_REV_SFX_1,
    audioId
  );
  if (DEBUG_OUTPUT) {
    std::cout << "Posted rev sound event, ID: " << revId << " Event ID: " << AK::EVENTS::PLAY_ENGINE_REV_SFX_1 << std::endl;
    if (revId == AK_INVALID_PLAYING_ID) {
      std::cout << "ERROR: Failed to play rev sound!" << std::endl;
    }
  }

  // Store the audio ID
  m_carAudioIds[car] = audioId;
  if (DEBUG_OUTPUT) {
    std::cout << "Stored car audio ID in map" << std::endl;
  }

  // Set initial volumes
  AK::SoundEngine::SetRTPCValue("Engine_Idle_Volume", 100.0f, audioId);
  AK::SoundEngine::SetRTPCValue("Engine_Rev_Volume", 0.0f, audioId);
  if (DEBUG_OUTPUT) {
    std::cout << "Set initial volumes" << std::endl;
  }

}

void AudioEngine::updateCarAudio(Car* car) {
  if (!m_audioEngine) return;
  auto it = m_carAudioIds.find(car);
  if (it == m_carAudioIds.end()) return;

  AkGameObjectID audioId = it->second;
  auto info = car->getDebugInfo();

  // Create sound position
  AkSoundPosition soundPos;

  float positionScale = 1.0f;

  // Position
  AkVector position;
  position.X = info.position.x * positionScale;
  position.Y = info.position.y * positionScale;
  position.Z = 0.0f;

  // Forward vector from car rotation 
  AkVector forward;
  forward.X = cos(info.angle);
  forward.Y = sin(info.angle);
  forward.Z = 0.0f;

  // Up vector (Z-up for 2D)
  AkVector up;
  up.X = 0.0f;
  up.Y = 0.0f;
  up.Z = 1.0f;

  soundPos.Set(position, forward, up);

  if (DEBUG_OUTPUT) {
    std::cout << "Setting car " << audioId << " position: ("
      << position.X << ", " << position.Y << ")" << std::endl;
  }

  AKRESULT result = AK::SoundEngine::SetPosition(audioId, soundPos);
  if (DEBUG_OUTPUT) {
    if (result != AK_Success) {
      std::cout << "Failed to set car position" << std::endl;
    }
  }

  // Continue with RTPC updates
  float speedRatio = abs(info.forwardSpeed) / (car->getProperties().maxSpeed/5.0f);
  updateCarEngineState(car, speedRatio);
}

void AudioEngine::updateCarEngineState(Car* car, float speedRatio) {
  auto it = m_carAudioIds.find(car);
  if (it == m_carAudioIds.end()) return;

  AkGameObjectID audioId = it->second;

  float idleVolume = 100.0f * (1.0f - speedRatio);
  float revVolume = 100.0f * speedRatio;

  AKRESULT idleResult = AK::SoundEngine::SetRTPCValue(
    "Engine_Idle_Volume",
    idleVolume,
    audioId
  );

  AKRESULT revResult = AK::SoundEngine::SetRTPCValue(
    "Engine_Rev_Volume",
    revVolume,
    audioId
  );

  if (DEBUG_OUTPUT) {
    std::cout << "Car " << audioId
      << " RTPC update - Result: Idle=" << idleResult
      << " Rev=" << revResult
      << " Values: Idle=" << idleVolume
      << " Rev=" << revVolume << std::endl;
  }
}

void AudioEngine::removeCarAudio(Car* car) {
  auto it = m_carAudioIds.find(car);
  if (it == m_carAudioIds.end()) return;

  AK::SoundEngine::UnregisterGameObj(it->second);

  m_carAudioIds.erase(it);
}
