// RaceTimer.cpp

#include "RaceTimer.h"
#include <iomanip>
#include <sstream>

RaceTimer::RaceTimer()
  : m_elapsedTime(0.0f)
  , m_isRunning(false) {
}

void RaceTimer::init(const char* fontPath, int fontSize) {
  m_font = std::make_unique<JAGEngine::SpriteFont>();
  m_font->init(fontPath, fontSize);
}

void RaceTimer::start() {
  m_isRunning = true;
}

void RaceTimer::stop() {
  m_isRunning = false;
}

void RaceTimer::reset() {
  m_elapsedTime = 0.0f;
  m_isRunning = false;
}

void RaceTimer::update(float deltaTime) {
  if (m_isRunning) {
    m_elapsedTime += deltaTime;
  }
}

std::string RaceTimer::formatTime() const {
  int hours = static_cast<int>(m_elapsedTime / 3600.0f);
  int minutes = static_cast<int>((m_elapsedTime / 60.0f)) % 60;
  int seconds = static_cast<int>(m_elapsedTime) % 60;
  int milliseconds = static_cast<int>((m_elapsedTime - std::floor(m_elapsedTime)) * 1000.0f);

  std::stringstream ss;

  // Only show hours if we've reached 1 hour
  if (hours > 0) {
    ss << std::setfill('0') << std::setw(2) << hours << ":";
  }

  ss << std::setfill('0') << std::setw(2) << minutes << ":"
    << std::setfill('0') << std::setw(2) << seconds << "."
    << std::setfill('0') << std::setw(3) << milliseconds;

  return ss.str();
}

void RaceTimer::draw(JAGEngine::SpriteBatch& batch, const JAGEngine::Camera2D& camera, int currentLap, int totalLaps) {
  if (!m_font || !m_font->getTextureID()) return;
  drawCenteredText(batch, camera, formatTime());
  if (currentLap >= 0 && totalLaps > 0) {
    drawLapCount(batch, camera, currentLap, totalLaps);
  }
}

void RaceTimer::drawCenteredText(JAGEngine::SpriteBatch& batch,
  const JAGEngine::Camera2D& camera,
  const std::string& text) {
  // Calculate top-middle position (assuming camera position is center of screen)
  glm::vec2 screenDims = camera.getScreenDimensions();
  glm::vec2 topMiddle(
    camera.getPosition().x,                    // Center X
    camera.getPosition().y + screenDims.y - 110.0f  // Top Y with small padding
  );

  m_font->draw(batch,
    text.c_str(),
    topMiddle,
    glm::vec2(2.0f),  // Smaller scale for timer display
    0.0f,
    JAGEngine::ColorRGBA8(255, 255, 255, 255),  // White color
    JAGEngine::Justification::MIDDLE);
}

void RaceTimer::drawLapCount(JAGEngine::SpriteBatch& batch,
  const JAGEngine::Camera2D& camera,
  int currentLap,
  int totalLaps) {
  if (!m_font) return;

  glm::vec2 screenDims = camera.getScreenDimensions();
  glm::vec2 pos(
    camera.getPosition().x,
    camera.getPosition().y + screenDims.y - 220.0f
  );

  std::string text = "Lap " + std::to_string(currentLap) + "/" + std::to_string(totalLaps);
  m_font->draw(batch,
    text.c_str(),
    pos,
    glm::vec2(2.0f),
    0.0f,
    JAGEngine::ColorRGBA8(255, 255, 255, 255),
    JAGEngine::Justification::MIDDLE);
}
