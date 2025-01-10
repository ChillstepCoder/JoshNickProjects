// AudioEngine.h

#pragma once
#include <memory>

namespace JAGEngine {
  class WWiseAudioEngine;
}

class AudioEngine {
public:
  AudioEngine();
  ~AudioEngine();

  bool init();
  void update();
  void cleanup();

  // Racing-specific audio functions
  void playEngineSound(float rpm, float load);
  void playTireSkidSound(float slipAmount);
  void playCollisionSound(float impactForce);
  void playBoostSound();
  void stopBoostSound();
  void playCountdownSound();
  void playLapCompletedSound();
  void playRaceStartSound();
  void playRaceFinishSound();
  void playCheckpointSound();

  // Sound state control
  void setEngineRPM(float rpm);
  void setCarSpeed(float speed);
  void setTireSurfaceType(int surfaceType);

  // Volume controls
  void setMasterVolume(float volume);
  void setEffectsVolume(float volume);
  void setMusicVolume(float volume);

  float getMasterVolume() { return m_masterVolume; }
  float getEffectsVolume() { return m_effectsVolume; }
  float getMusicVolume() { return m_musicVolume; }

private:
  std::unique_ptr<JAGEngine::WWiseAudioEngine> m_audioEngine;
  bool m_isBoostPlaying;
  bool m_isEnginePlaying;
  float m_currentRPM;
  float m_currentSpeed;
  int m_currentSurfaceType;
  float m_masterVolume;
  float m_effectsVolume;
  float m_musicVolume;
};
