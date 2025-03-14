// AudioEngine.h

#pragma once
#include <memory>
#include <AK/SoundEngine/Common/AkTypes.h>
#include <unordered_map>
#include "RacingAudioDefs.h"
#include <array>
#include "PhysicsSystem.h"

class Car;

static constexpr size_t NUM_SURFACES = 5;

struct Vec2 {
  float x, y;
  Vec2(float x_ = 0, float y_ = 0) : x(x_), y(y_) {}
};

namespace JAGEngine {
  class WWiseAudioEngine;
}

struct MusicTrack {
  const char* name;
  AkUniqueID playEventId;
  AkUniqueID stopEventId;
};

static const std::vector<MusicTrack> AVAILABLE_MUSIC = {
    {"Cyber Chase", AK::EVENTS::PLAY_CYBERCHASE, AK::EVENTS::STOP_CYBERCHASE},
    {"Hyper Loop", AK::EVENTS::PLAY_HYPERLOOP, AK::EVENTS::STOP_HYPERLOOP}
};

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
  void playCountdownBeep();
  void playCountdownStart();
  void playLapCompletedSound();
  void playRaceStartSound();
  void playRaceFinishSound();
  void playCheckpointSound();
  void playMusicTrack(AkUniqueID trackId);
  void stopMusicTrack(AkUniqueID trackId);

  void handleCarCollision(const PhysicsSystem::CollisionInfo& collision);

  // Sound state control
  void setEngineRPM(float rpm);
  void setCarSpeed(float speed);
  void setTireSurfaceType(int surfaceType);
  void updateTireSkidSound(Car* car, float speedRatio, float driftState);
  void updateScrapeSound(Car* car);

  void setDefaultListener(const Vec2& position, float rotation); // not used ATM
  void setObjectPosition(AkGameObjectID id, const Vec2& position) const;

  void initializeCarAudio(Car* car);
  void updateCarAudio(Car* car, const Vec2& listenerPos);
  void removeCarAudio(Car* car);
  void resetNextCarAudioId();

  // Volume controls
  void setMasterVolume(float volume);
  void setEffectsVolume(float volume);
  void setMusicVolume(float volume);
  void setDopplerIntensity(float intensity) { m_dopplerIntensity = intensity; }

  float getMasterVolume() { return m_masterVolume; }
  float getEffectsVolume() { return m_effectsVolume; }
  float getMusicVolume() { return m_musicVolume; }
  const std::vector<MusicTrack>& getAvailableTracks() const { return AVAILABLE_MUSIC; }

  JAGEngine::WWiseAudioEngine* getWWiseEngine() { return m_audioEngine.get(); }

private:
  struct CarAudioData {
    AkGameObjectID audioId;
    Vec2 lastPosition;
    float lastDistanceToListener;
  };
  std::unordered_map<Car*, CarAudioData> m_carAudioData;
  float m_dopplerIntensity = 1.0f;

  static constexpr bool DEBUG_OUTPUT = false;

  std::unique_ptr<JAGEngine::WWiseAudioEngine> m_audioEngine;
  void updateCarEngineState(Car* car, float speedRatio);
  std::unordered_map<Car*, AkGameObjectID> m_carAudioIds;
  std::unordered_map<Car*, bool> m_isScrapePlaying;

  bool m_isBoostPlaying;
  bool m_isEnginePlaying;
  float m_currentRPM;
  float m_currentSpeed;
  int m_currentSurfaceType;
  float m_masterVolume;
  float m_effectsVolume;
  float m_musicVolume;
  AkGameObjectID m_nextCarAudioId = 1;

};
