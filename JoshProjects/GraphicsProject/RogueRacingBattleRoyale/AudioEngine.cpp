// AudioEngine.cpp

#include "AudioEngine.h"
#include "Car.h"
#include <JAGEngine/WWiseAudioEngine.h>
#include <AK/Plugin/AkImpacterSourceFactory.h>
#include <iostream>
#include <algorithm>
#include <cmath>
#include "PhysicsCategories.h"
#include "ObjectProperties.h"
#include "PlaceableObject.h"
#include <Box2D/box2d.h>


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
  std::cout << "Registering audio game objects...\n";

  // Register Countdown game object
  AkGameObjectID countdownID = RacingAudio::GAME_OBJECT_COUNTDOWN;
  AKRESULT result = AK::SoundEngine::RegisterGameObj(countdownID, "Countdown");
  if (result != AK_Success) {
    std::cout << "Failed to register countdown game object. Result: " << result << std::endl;
    return false;
  }

  // Register Music game object
  AkGameObjectID musicId = RacingAudio::GAME_OBJECT_MUSIC;
  result = AK::SoundEngine::RegisterGameObj(musicId, "Music");
  if (result != AK_Success) {
    std::cout << "Failed to register music game object. Result: " << result << std::endl;
    return false;
  }

  // Register Impacts game object
  AkGameObjectID impactsId = RacingAudio::GAME_OBJECT_IMPACTS;
  result = AK::SoundEngine::RegisterGameObj(impactsId, "Impacts");
  if (result != AK_Success) {
    std::cout << "Failed to register impacts game object. Result: " << result << std::endl;
    return false;
  }

  std::cout << "Game objects registered successfully\n";

  // Set initial volumes
  m_masterVolume = 1.0f;
  m_effectsVolume = 1.0f;
  m_musicVolume = 1.0f;

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
  if (!m_audioEngine || !m_audioEngine->isInitialized()) return;

  // Post the collision event
  AK::SoundEngine::PostEvent(
    AK::EVENTS::PLAY_CAR_COLLISION,
    RacingAudio::GAME_OBJECT_IMPACTS
  );
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

void AudioEngine::playMusicTrack(AkUniqueID trackId) {
  if (!m_audioEngine || !m_audioEngine->isInitialized()) {
    std::cout << "Audio engine not initialized for music playback" << std::endl;
    return;
  }

  AkPlayingID playingId = AK::SoundEngine::PostEvent(
    trackId,
    RacingAudio::GAME_OBJECT_MUSIC
  );

  if (playingId == AK_INVALID_PLAYING_ID) {
    std::cout << "Failed to play music track ID: " << trackId << std::endl;
  }
  else {
    std::cout << "Started playing music track ID: " << trackId << std::endl;
  }
}

void AudioEngine::stopMusicTrack(AkUniqueID trackId) {
  if (!m_audioEngine || !m_audioEngine->isInitialized()) return;
  AK::SoundEngine::PostEvent(trackId, RacingAudio::GAME_OBJECT_MUSIC);
}

void AudioEngine::handleCarCollision(const PhysicsSystem::CollisionInfo& collision) {
  if (!m_audioEngine || !m_audioEngine->isInitialized())
    return;

  // Skip processing if neither car exists
  if (!collision.carA && !collision.carB) return;

  // Get audio IDs, handling case where one car might be null
  AkGameObjectID audioIdA = collision.carA ? collision.carA->getAudioId() : 0;
  AkGameObjectID audioIdB = collision.carB ? collision.carB->getAudioId() : 0;

  // Get a valid audio ID to play sounds on
  AkGameObjectID primaryAudioId = audioIdA ? audioIdA : audioIdB;
  if (primaryAudioId == 0) return;

  // Use the normalized values from collision info directly
  float rtpcSpeed = collision.speed * 100; // Scale to 0-100 range
  float rtpcMass = collision.mass * 100;   // Scale to 0-100 range

  // Set RTPC values
  AK::SoundEngine::SetRTPCValue(AK::GAME_PARAMETERS::COLLISION_VELOCITY, rtpcSpeed, primaryAudioId);
  AK::SoundEngine::SetRTPCValue(AK::GAME_PARAMETERS::COLLISION_MASS, rtpcMass, primaryAudioId);

  // Debug print object types
  std::cout << "Collision objects: " << std::endl;
  if (collision.objectA) {
    std::cout << "Object A type: " << static_cast<int>(collision.objectA->getObjectType()) << std::endl;
  }
  if (collision.objectB) {
    std::cout << "Object B type: " << static_cast<int>(collision.objectB->getObjectType()) << std::endl;
  }

  // First check for traffic cone collisions
  bool isTrafficCone = false;
  if (collision.objectA && collision.objectA->getObjectType() == ObjectType::TrafficCone) {
    std::cout << "Traffic cone collision detected (Object A)" << std::endl;
    isTrafficCone = true;
  }
  else if (collision.objectB && collision.objectB->getObjectType() == ObjectType::TrafficCone) {
    std::cout << "Traffic cone collision detected (Object B)" << std::endl;
    isTrafficCone = true;
  }

  // Choose which collision sound to play based on object type and speed
  if (isTrafficCone) {
    std::cout << "Playing traffic cone sound" << std::endl;
    AK::SoundEngine::PostEvent(AK::EVENTS::PLAY_TRAFFICCONE_SFX, primaryAudioId);
  }
  else if (rtpcSpeed > 50.0f) {
    std::cout << "Playing glass break sound" << std::endl;
    AK::SoundEngine::PostEvent(AK::EVENTS::PLAY_CAR_COLLISIONGLASS, primaryAudioId);
  }
  else {
    std::cout << "Playing regular collision sound" << std::endl;
    AK::SoundEngine::PostEvent(AK::EVENTS::PLAY_CAR_COLLISION, primaryAudioId);
  }
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

void AudioEngine::updateTireSkidSound(Car* car, float speedRatio, float driftState) {
  auto it = m_carAudioIds.find(car);
  if (it == m_carAudioIds.end()) return;

  AkGameObjectID audioId = it->second;
  static std::unordered_map<Car*, bool> isSkidPlaying;

  // Calculate surface ratios
  std::array<int, 5> surfaceCounts = { 0, 0, 0, 0, 0 };
  const auto& wheelStates = car->getWheelStates();
  for (const auto& state : wheelStates) {
    surfaceCounts[static_cast<int>(state.surface)]++;
  }

  float roadRatio = (surfaceCounts[0] + surfaceCounts[1] * 0.5f) / 4.0f;
  float dirtRatio = (surfaceCounts[2] + surfaceCounts[1] * 0.5f + surfaceCounts[3] * 0.5f) / 4.0f;
  float grassRatio = (surfaceCounts[4] + surfaceCounts[3] * 0.5f) / 4.0f;

  if (dirtRatio > 0.0f || grassRatio > 0.0f || driftState > 0.0f) {
    if (!isSkidPlaying[car]) {
      AK::SoundEngine::PostEvent(AK::EVENTS::PLAY_CAR_TIRE_SKID_1, audioId);
      isSkidPlaying[car] = true;
    }

    // Road skid with master volume
    float skidVolume = std::pow(driftState, 2.0f) * 100.0f;
    float roadVolume = skidVolume * roadRatio;
    AK::SoundEngine::SetRTPCValue("Tire_Road_Volume", roadVolume, audioId);
    AK::SoundEngine::SetRTPCValue("Tire_Skid_Volume", skidVolume, audioId);

    // Speed-based volume for dirt/grass (50% of max)
    float speedVolume = 50.0f * std::sqrt(speedRatio);
    // Additional volume when drifting (50% of max)
    float addSkidVolume = 50.0f * std::pow(driftState, 2.0f);

    AK::SoundEngine::SetRTPCValue("Tire_Dirt_Volume", (speedVolume + addSkidVolume) * dirtRatio, audioId);
    AK::SoundEngine::SetRTPCValue("Tire_Grass_Volume", (speedVolume + addSkidVolume) * grassRatio, audioId);
    AK::SoundEngine::SetRTPCValue("Tire_Skid_Pitch", speedRatio * 100.0f, audioId);
  }
  else if (isSkidPlaying[car]) {
    AK::SoundEngine::PostEvent(AK::EVENTS::STOP_CAR_TIRE_SKID_1, audioId);
    isSkidPlaying[car] = false;
  }
}

void AudioEngine::updateScrapeSound(Car* car) {
  if (!car) return;
  AkGameObjectID audioId = car->getAudioId();

  // Check if the car is scraping
  bool scraping = car->isScrapingBarrier();

  if (scraping && !m_isScrapePlaying[car]) {
    // Start the scrape loop if not already playing
    AK::SoundEngine::PostEvent(AK::EVENTS::PLAY_SCRAPE_SFX, audioId);
    m_isScrapePlaying[car] = true;
  }
  else if (!scraping && m_isScrapePlaying[car]) {
    // Stop the scrape loop if it is playing
    AK::SoundEngine::PostEvent(AK::EVENTS::STOP_SCRAPE_SFX, audioId);
    m_isScrapePlaying[car] = false;
  }
}

void AudioEngine::setDefaultListener(const Vec2& position, float rotation) {
  AkListenerPosition listenerPos;

  // Position with scale
  float positionScale = 1.0f;

  AkVector position3D;
  position3D.X = position.x * positionScale;
  position3D.Y = -position.y * positionScale;
  position3D.Z = 0.0f;

  // Forward vector from rotation
  AkVector orientFront;
  orientFront.X = cos(-rotation);
  orientFront.Y = sin(-rotation);
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

  auto info = car->getDebugInfo();
  CarAudioData audioData;
  audioData.audioId = car->getAudioId();
  audioData.lastPosition = Vec2(info.position.x, info.position.y);
  audioData.lastDistanceToListener = 0.0f;
  m_carAudioData[car] = audioData;

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

  // Generate random offsets for idle and rev sounds
  float idleOffset = static_cast<float>(rand()) / RAND_MAX * 3.0f;  // 0 to 3 seconds
  float revOffset = static_cast<float>(rand()) / RAND_MAX * 3.0f;   // 0 to 3 seconds

  // Set random seek positions for both sounds
  AkTimeMs idleSeekTime = static_cast<AkTimeMs>(idleOffset * 1000);  // Convert to milliseconds
  AkTimeMs revSeekTime = static_cast<AkTimeMs>(revOffset * 1000);    // Convert to milliseconds

  // Create playback position structures
  AkSegmentInfo idleSegment;
  idleSegment.iCurrentPosition = idleSeekTime;
  AkSegmentInfo revSegment;
  revSegment.iCurrentPosition = revSeekTime;

  // Start playing the idle sound (loops infinitely)
  AkPlayingID idleId = AK::SoundEngine::PostEvent(
    AK::EVENTS::PLAY_ENGINE_IDLE_SFX_1,
    audioId
  );
  if (DEBUG_OUTPUT) {
    std::cout << "Posted idle sound event, ID: " << idleId
      << " Event ID: " << AK::EVENTS::PLAY_ENGINE_IDLE_SFX_1
      << " Offset: " << idleOffset << "s" << std::endl;
    if (idleId == AK_INVALID_PLAYING_ID) {
      std::cout << "ERROR: Failed to play idle sound!" << std::endl;
    }
  }

  // Set initial seek time for idle
  if (idleId != AK_INVALID_PLAYING_ID) {
    AK::SoundEngine::SeekOnEvent(
      AK::EVENTS::PLAY_ENGINE_IDLE_SFX_1,
      audioId,
      idleSeekTime,
      false  // seeking from beginning
    );
  }

  // Start playing the rev sound (at 0 volume initially)
  AkPlayingID revId = AK::SoundEngine::PostEvent(
    AK::EVENTS::PLAY_ENGINE_REV_SFX_1,
    audioId
  );
  if (DEBUG_OUTPUT) {
    std::cout << "Posted rev sound event, ID: " << revId
      << " Event ID: " << AK::EVENTS::PLAY_ENGINE_REV_SFX_1
      << " Offset: " << revOffset << "s" << std::endl;
    if (revId == AK_INVALID_PLAYING_ID) {
      std::cout << "ERROR: Failed to play rev sound!" << std::endl;
    }
  }

  // Set initial seek time for rev
  if (revId != AK_INVALID_PLAYING_ID) {
    AK::SoundEngine::SeekOnEvent(
      AK::EVENTS::PLAY_ENGINE_REV_SFX_1,
      audioId,
      revSeekTime,
      false  // seeking from beginning
    );
  }

  // Store the audio ID
  m_carAudioIds[car] = audioId;
  if (DEBUG_OUTPUT) {
    std::cout << "Stored car audio ID in map" << std::endl;
  }

  // Set initial volumes
  AK::SoundEngine::SetRTPCValue("Engine_Idle_Volume", 100.0f, audioId);
  AK::SoundEngine::SetRTPCValue("Engine_Rev_Volume", 0.0f, audioId);

  // Set initial pitch value for rev sound
  AK::SoundEngine::SetRTPCValue("Engine_Rev_Pitch", 100.0f, audioId);  // Base pitch

  if (DEBUG_OUTPUT) {
    std::cout << "Set initial volumes and pitch" << std::endl;
  }
}

void AudioEngine::updateCarAudio(Car* car, const Vec2& listenerPos) {
  // Basic checks
  if (!m_audioEngine) return;

  // Get audio data
  auto audioIt = m_carAudioIds.find(car);
  auto dataIt = m_carAudioData.find(car);
  if (audioIt == m_carAudioIds.end() || dataIt == m_carAudioData.end()) return;

  AkGameObjectID audioId = audioIt->second;
  auto& audioData = dataIt->second;
  auto info = car->getDebugInfo();

  // Get current car position
  Vec2 currentPos(info.position.x, info.position.y);

  // Calculate current distance to listener
  float currentDistance = std::sqrt(
    std::pow(currentPos.x - listenerPos.x, 2) +
    std::pow(currentPos.y - listenerPos.y, 2)
  );

  // Calculate highpass filter value based on distance (0-1 range)
  float highpassValue = std::min<float>(currentDistance / 500.0f, 1.0f) * 100.0f;
  AK::SoundEngine::SetRTPCValue("Engine_Highpass_Filter", highpassValue, audioId);

  if (DEBUG_OUTPUT) {
    std::cout << "Distance: " << currentDistance
      << " Highpass: " << highpassValue << std::endl;
  }

  // Calculate Doppler effect
  float dopplerValue = 50.0f;  // No pitch shift by default
  if (audioData.lastDistanceToListener > 0.0f) {
    float distanceChange = currentDistance - audioData.lastDistanceToListener;
    float relativeVelocity = -distanceChange * 60.0f;
    relativeVelocity *= m_dopplerIntensity;
    dopplerValue = 50.0f + (relativeVelocity * 50.0f);
    dopplerValue = std::max<float>(0.0f, std::min<float>(100.0f, dopplerValue));
  }

  // Store current values for next update
  audioData.lastPosition = currentPos;
  audioData.lastDistanceToListener = currentDistance;

  // Update Doppler RTPC
  AK::SoundEngine::SetRTPCValue(
    "Engine_Rev_Doppler_Effect",
    dopplerValue,
    audioId
  );

  // Create sound position
  AkSoundPosition soundPos;
  float positionScale = 1.0f;

  // Position
  AkVector position;
  position.X = info.position.x * positionScale;
  position.Y = -info.position.y * positionScale;
  position.Z = 0.0f;

  // Forward vector
  AkVector forward;
  forward.X = cos(-info.angle);
  forward.Y = sin(-info.angle);
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
  float speedRatio = abs(info.forwardSpeed) / (500.0f);
  updateCarEngineState(car, speedRatio);


  // Update scraping flag using the C API
  bool scraping = false;
  b2BodyId bodyId = car->getDebugInfo().bodyId;

  // Get our car pointer from the body’s user data.
  void* carUserData = b2Body_GetUserData(bodyId);

  // Get contact data for this body.
  int capacity = b2Body_GetContactCapacity(bodyId);
  if (capacity > 0) {
    // Allocate an array for contacts.
    std::vector<b2ContactData> contacts(capacity);
    int contactCount = b2Body_GetContactData(bodyId, contacts.data(), capacity);

    for (int i = 0; i < contactCount; i++) {
      b2ContactData contact = contacts[i];
      // Use the manifold's pointCount as a proxy for "touching".
      if (contact.manifold.pointCount == 0) {
        continue;
      }

      b2ShapeId shapeA = contact.shapeIdA;
      b2ShapeId shapeB = contact.shapeIdB;
      b2BodyId bodyA = b2Shape_GetBody(shapeA);
      b2BodyId bodyB = b2Shape_GetBody(shapeB);

      // Retrieve user data for both bodies.
      void* userDataA = b2Body_GetUserData(bodyA);
      void* userDataB = b2Body_GetUserData(bodyB);

      b2ShapeId otherShape;
      // Determine which shape is _not_ attached to our car.
      if (userDataA == carUserData) {
        otherShape = shapeB;
      }
      else if (userDataB == carUserData) {
        otherShape = shapeA;
      }
      else {
        // Neither shape belongs to our car; skip.
        continue;
      }

      // Get the filter data from the "other" shape.
      b2Filter filter = b2Shape_GetFilter(otherShape);
      // If the filter's category includes the barrier flag, we are scraping.
      if (filter.categoryBits & CATEGORY_BARRIER) {
        scraping = true;
        break;
      }
    }
  }
  car->setScrapingBarrier(scraping);

  // Update the scrape sound
  updateScrapeSound(car);
}

void AudioEngine::updateCarEngineState(Car* car, float speedRatio) {
  auto it = m_carAudioIds.find(car);
  if (it == m_carAudioIds.end()) return;
  AkGameObjectID audioId = it->second;

  speedRatio = std::min<float>(1.0f, std::max<float>(0.0f, speedRatio));

  // Start idle at 50%, surge up to 150% at low speeds, then drop to near 0 at high speeds 
  float idleMultiplier;
  if (speedRatio < 0.2f) {
    idleMultiplier = 0.5f + (speedRatio * 5.0f); // 0.5 to 1.5
  }
  else {
    idleMultiplier = 1.5f - std::pow(speedRatio, 0.7f) * 1.5f; // Steep drop
  }
  
  // Only apply multiplier to idle volume
  float idleVolume = (100.0f * (1.0f - std::sqrt(speedRatio))) * idleMultiplier;
  idleVolume = std::min<float>(100.0f, std::max<float>(0.0f, idleVolume));
  float revVolume = 100.0f * std::sqrt(speedRatio); // Rev unchanged
  revVolume = std::min<float>(100.0f, revVolume);

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

  float pitchValue = speedRatio * 100.0f;
  AK::SoundEngine::SetRTPCValue("Engine_Rev_Pitch", pitchValue, audioId);

  updateTireSkidSound(car, speedRatio, car->getProperties().driftState);

  if (DEBUG_OUTPUT) {
    std::cout << "Car " << audioId
      << " RTPC update - Result: Idle=" << idleResult
      << " Rev=" << revResult
      << " Values: Idle=" << idleVolume
      << " Rev=" << revVolume
      << " Pitch=" << pitchValue << std::endl;
  }
}

void AudioEngine::removeCarAudio(Car* car) {
  auto it = m_carAudioIds.find(car);
  if (it == m_carAudioIds.end())
    return;

  AkGameObjectID audioId = it->second;

  // Stop all sounds on this game object.
  AK::SoundEngine::StopAll(audioId);
  // Ensure the commands are flushed.
  AK::SoundEngine::RenderAudio();
  // Unregister the game object.
  AK::SoundEngine::UnregisterGameObj(audioId);

  m_carAudioIds.erase(it);

  if (DEBUG_OUTPUT) {
    std::cout << "Removed audio for car " << audioId << std::endl;
  }
}


