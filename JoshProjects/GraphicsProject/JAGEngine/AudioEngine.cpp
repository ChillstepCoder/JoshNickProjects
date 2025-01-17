// AudioEngine.cpp

#include "AudioEngine.h"
#include "JAGErrors.h"

namespace JAGEngine {

  // Initialize static members
  int SoundEffect::nextChannel = 0;
  int SoundEffect::totalChannels = 8; // Default to 8 channels, can be changed later

  void SoundEffect::play(int loops /* = 0 */) {
    if (Mix_PlayChannel(-1, m_chunk, loops) == -1) {
      if (Mix_PlayChannel(nextChannel, m_chunk, loops) == -1) {
        // If it still fails, log the error but don't crash
        fatalError("Mix_PlayChannel error: " + std::string(Mix_GetError()));
      }
      // Move to the next channel
      nextChannel = (nextChannel + 1) % totalChannels;
    }
  }

  void Music::play(int loops /* = -1 */) {
    Mix_PlayMusic(m_music, loops);
  }

  void Music::pause() {
    Mix_PauseMusic();
  }

  void Music::stop() {
    Mix_HaltMusic();
  }

  void Music::resume() {
    Mix_ResumeMusic();
  }

AudioEngine::AudioEngine() {
  //empty
}

AudioEngine::~AudioEngine() {
  destroy();
}

void AudioEngine::init() {

  if (m_isInitialized) {
    fatalError("Tried to initialize Audio Engine twice!");
  }

  if (Mix_Init(MIX_INIT_MP3 | MIX_INIT_OGG) == -1) {
    fatalError("Mix_Init error: " + std::string(Mix_GetError()));
  }
  if (Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, 2, 1024) == -1) {
    fatalError("Mix_OpenAudio error: " + std::string(Mix_GetError()));
  }
  setChannels(8); // Set default number of channels
  m_isInitialized = true;
}

void AudioEngine::setChannels(int channels) {
  SoundEffect::totalChannels = Mix_AllocateChannels(channels);
  SoundEffect::nextChannel = 0;
}

void AudioEngine::destroy() {
  if (m_isInitialized) {
    m_isInitialized = false;

    for (auto& it : m_effectMap) {
      Mix_FreeChunk(it.second);
    }

    for (auto& it : m_musicMap) {
      Mix_FreeMusic(it.second);
    }

    m_effectMap.clear();
    m_musicMap.clear();

    Mix_CloseAudio();
    Mix_Quit();
  }
}

SoundEffect AudioEngine::loadSoundEffect(const std::string& filePath) {
  //try to find the audio in the cache
  auto it = m_effectMap.find(filePath);

  SoundEffect effect;

  if (it == m_effectMap.end()) {
    Mix_Chunk* chunk = Mix_LoadWAV(filePath.c_str());
    if (chunk == nullptr) {
      fatalError("Mix_LoadWAV error: " + std::string(Mix_GetError()));
    }

    effect.m_chunk = chunk;
    m_effectMap[filePath] = chunk;

  }
  else {
    //its already cached
    effect.m_chunk = it->second;
  }

  return effect;
}

Music AudioEngine::loadMusic(const std::string& filePath) {
  //try to find the audio in the cache
  auto it = m_musicMap.find(filePath);

  Music music;

  if (it == m_musicMap.end()) {
    Mix_Music* mixMusic = Mix_LoadMUS(filePath.c_str());
    if (mixMusic == nullptr) {
      fatalError("Mix_LoadMUS error: " + std::string(Mix_GetError()));
    }

    music.m_music = mixMusic;
    m_musicMap[filePath] = mixMusic;

  }
  else {
    //its already cached
    music.m_music = it->second;
  }

  return music;
}

}
