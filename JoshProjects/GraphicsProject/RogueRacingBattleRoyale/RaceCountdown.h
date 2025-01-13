// RaceCountdown.h

#pragma once
#include <JAGEngine/SpriteFont.h>
#include <JAGEngine/SpriteBatch.h>
#include <JAGEngine/Camera2D.h>
#include <functional>
#include "AudioEngine.h"

class RaceCountdown {
public:
  RaceCountdown();
  ~RaceCountdown();

  // Initialize with font path and size
  void init(const char* fontPath, int fontSize);

  // Start the countdown sequence
  void startCountdown();

  // Update countdown state
  void update(float deltaTime);
  void reset();

  // Draw countdown
  void draw(JAGEngine::SpriteBatch& batch, const JAGEngine::Camera2D& camera);

  // Getters
  bool isCountingDown() const { return m_isCountingDown; }
  bool isWaitingToStart() const { return !m_isCountingDown && !m_hasFinished; }
  bool hasFinished() const { return m_hasFinished; }
  std::string getText() const;

  // Setters
  void setFont(std::unique_ptr<JAGEngine::SpriteFont> font);

  // Event callbacks
  void setOnCountdownStart(std::function<void()> callback) { m_onCountdownStart = callback; }
  void setOnCountdownComplete(std::function<void()> callback) { m_onCountdownComplete = callback; }

private:
  std::unique_ptr<JAGEngine::SpriteFont> m_font;
  float m_timer;
  bool m_isCountingDown;
  bool m_hasFinished;
  AudioEngine* m_audioEngine;

  // Countdown settings
  static constexpr float COUNTDOWN_START = 3.0f;
  static constexpr float COUNT_DURATION = 1.0f;

  // Callbacks
  std::function<void()> m_onCountdownStart;
  std::function<void()> m_onCountdownComplete;
};
