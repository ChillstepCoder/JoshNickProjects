// RaceTimer.h

#pragma once
#include <JAGEngine/SpriteFont.h>
#include <JAGEngine/SpriteBatch.h>
#include <JAGEngine/Camera2D.h>
#include <memory>
#include <string>

class RaceTimer {
public:
  RaceTimer();
  void init(const char* fontPath, int fontSize);
  void setFont(JAGEngine::SpriteFont* font) { m_font.reset(font); }
  void start();
  void stop();
  void reset();
  void update(float deltaTime);
  void draw(JAGEngine::SpriteBatch& batch, const JAGEngine::Camera2D& camera, int currentLap = -1, int totalLaps = -1);
  bool isRunning() const { return m_isRunning; }

private:
  std::unique_ptr<JAGEngine::SpriteFont> m_font;
  float m_elapsedTime;
  bool m_isRunning;
  static constexpr float MS_PRECISION = 100.0f;

  std::string formatTime() const;
  void drawCenteredText(JAGEngine::SpriteBatch& batch, const JAGEngine::Camera2D& camera, const std::string& text);
  void drawLapCount(JAGEngine::SpriteBatch& batch, const JAGEngine::Camera2D& camera, int currentLap, int totalLaps);
};
