// AudioEngine.cpp

#include "AudioEngine.h"
#include "RacingAudioDefs.h"
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

  // Initialize racing-specific audio settings here
  // Set up RTPC parameters for engine, tire sounds, etc.
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

  // Apply volumes to WWise - use explicit RTPC IDs if defined in Wwise_IDs.h
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
  if (m_audioEngine) {
    AkPlayingID playingID = AK::SoundEngine::PostEvent(
      AK::EVENTS::PLAY_COUNTDOWN_SFX_1,
      RacingAudio::GAME_OBJECT_COUNTDOWN
    );
  }
}

void AudioEngine::playCountdownStart() {
  if (m_audioEngine) {
    AkPlayingID playingID = AK::SoundEngine::PostEvent(
      AK::EVENTS::PLAY_COUNTDOWN_SFX_2,
      RacingAudio::GAME_OBJECT_COUNTDOWN
    );
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

void AudioEngine::initCarAudio(AkGameObjectID carId) {
  if (!m_audioEngine) return;

  // Register the car as a game object if not already registered
  AK::SoundEngine::RegisterGameObj(carId, "Car");

  // Start playing the idle sound
  AK::SoundEngine::PostEvent(AK::EVENTS::PLAY_ENGINE_IDLE_SFX_1, carId);
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

