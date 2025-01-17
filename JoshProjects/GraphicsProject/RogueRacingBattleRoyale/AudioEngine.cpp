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
  m_currentRPM = rpm;
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
  if (!m_audioEngine) return;

  AkListenerPosition listenerPos;
  AkVector position3D;
  position3D.X = position.x;
  position3D.Y = position.y;
  position3D.Z = 0.0f;

  // Create orientation vectors
  AkVector orientFront;
  orientFront.X = cosf(rotation);
  orientFront.Y = sinf(rotation);
  orientFront.Z = 0.0f;

  AkVector orientTop;
  orientTop.X = 0.0f;
  orientTop.Y = 0.0f;
  orientTop.Z = 1.0f;

  // Set the position and orientation
  listenerPos.Set(position3D, orientFront, orientTop);

  // Use RacingAudio namespace
  AK::SoundEngine::SetDefaultListeners(&RacingAudio::LISTENER_ID, 1);
  AK::SoundEngine::SetPosition(RacingAudio::LISTENER_ID, listenerPos);
}

void AudioEngine::setObjectPosition(AkGameObjectID id, const Vec2& position) const {
  if (!m_audioEngine) return;

  AkSoundPosition soundPos;
  AkVector position3D;
  position3D.X = position.x;
  position3D.Y = position.y;
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
  if (!m_audioEngine || !car) return;

  AkGameObjectID audioId = car->getAudioId();

  // Register the car's game object
  AKRESULT result = AK::SoundEngine::RegisterGameObj(audioId, "CarEngine");
  if (result != AK_Success) {
    std::cout << "Failed to register car audio object. Result: " << result << std::endl;
    return;
  }

  // Start playing the idle sound
  AK::SoundEngine::PostEvent(
    AK::EVENTS::PLAY_ENGINE_IDLE_SFX_1,
    audioId
  );

  // Start playing the rev sound
  AK::SoundEngine::PostEvent(
    AK::EVENTS::PLAY_ENGINE_REV_SFX_1,
    audioId
  );

  // Store the audio ID
  m_carAudioIds[car] = audioId;

  // Set initial volumes
  AK::SoundEngine::SetRTPCValue("Engine_Idle_Volume", 100.0f, audioId);
  AK::SoundEngine::SetRTPCValue("Engine_Rev_Volume", 0.0f, audioId);
}

void AudioEngine::updateCarAudio(Car* car) {
  if (!m_audioEngine || !car) return;

  auto it = m_carAudioIds.find(car);
  if (it == m_carAudioIds.end()) return;

  AkGameObjectID audioId = it->second;
  auto info = car->getDebugInfo();

  // Update position relative to listener
  if (info.position.x != 0.0f || info.position.y != 0.0f) {
    AkSoundPosition soundPos;

    // Position
    AkVector position;
    position.X = info.position.x;
    position.Y = info.position.y;
    position.Z = 0.0f;

    // Forward vector
    AkVector forward;
    forward.X = cos(info.angle);
    forward.Y = sin(info.angle);
    forward.Z = 0.0f;

    // Up vector
    AkVector up;
    up.X = 0.0f;
    up.Y = 0.0f;
    up.Z = 1.0f;

    soundPos.Set(position, forward, up);
    AK::SoundEngine::SetPosition(audioId, soundPos);
  }

  // Update engine state
  float speedRatio = abs(info.forwardSpeed) / car->getProperties().maxSpeed;
  updateCarEngineState(car, speedRatio);
}

void AudioEngine::updateCarEngineState(Car* car, float speedRatio) {
  auto it = m_carAudioIds.find(car);
  if (it == m_carAudioIds.end()) return;

  AkGameObjectID audioId = it->second;

  // Update engine sound parameters
  float idleVolume = 100.0f * (1.0f - speedRatio);
  float revVolume = 100.0f * speedRatio;

  AK::SoundEngine::SetRTPCValue(AK::GAME_PARAMETERS::ENGINE_IDLE_VOLUME, idleVolume, audioId);
  AK::SoundEngine::SetRTPCValue(AK::GAME_PARAMETERS::ENGINE_REV_VOLUME, revVolume, audioId);
}

void AudioEngine::removeCarAudio(Car* car) {
  auto it = m_carAudioIds.find(car);
  if (it == m_carAudioIds.end()) return;

  AK::SoundEngine::UnregisterGameObj(it->second);

  m_carAudioIds.erase(it);
}
