// RaceCountdown.cpp
#include "RaceCountdown.h"
#include <AK/SoundEngine/Common/AkSoundEngine.h>
#include "RacingAudioDefs.h"
#include "Wwise_IDs.h"

RaceCountdown::RaceCountdown()
  : m_timer(0.0f)
  , m_isCountingDown(false)
  , m_hasFinished(false)
  , m_font(nullptr)
  , m_audioEngine(nullptr) {
}

RaceCountdown::~RaceCountdown() {
}

void RaceCountdown::init(const char* fontPath, int fontSize) {
  m_font = std::make_unique<JAGEngine::SpriteFont>();
  m_font->init(fontPath, fontSize);
  GLuint textureID = m_font->getTextureID();
  //std::cout << "RaceCountdown font initialized. Texture ID: " << textureID << std::endl;

  // Verify texture is valid
  GLint width = 0;
  glBindTexture(GL_TEXTURE_2D, textureID);
  glGetTextureLevelParameteriv(textureID, 0, GL_TEXTURE_WIDTH, &width);
  //std::cout << "Font texture width: " << width << std::endl;
}

void RaceCountdown::startCountdown() {
  m_isCountingDown = true;
  m_hasFinished = false;
  m_timer = COUNTDOWN_START;
  if (m_onCountdownStart) m_onCountdownStart();
}

void RaceCountdown::update(float deltaTime) {
  if (m_isCountingDown) {
    float oldTime = m_timer;
    m_timer -= deltaTime;
    int oldCount = static_cast<int>(std::ceil(oldTime));
    int newCount = static_cast<int>(std::ceil(m_timer));

    // Check if we need to play the initial countdown sound
    if (oldTime == COUNTDOWN_START) {
      std::cout << "Playing countdown beep for start (3)" << std::endl;
      AkPlayingID playingID = AK::SoundEngine::PostEvent(
        AK::EVENTS::PLAY_COUNTDOWN_SFX_1,
        RacingAudio::GAME_OBJECT_COUNTDOWN
      );
      std::cout << "PostEvent ID: " << playingID
        << (playingID == AK_INVALID_PLAYING_ID ? " (INVALID)" : " (Valid)")
        //<< " Game object active: "
        //<< (AK::SoundEngine::GetIsGameObjectActive(RacingAudio::GAME_OBJECT_COUNTDOWN) ? "Yes" : "No")
        << std::endl;
    }

    // Check if count changed
    if (oldCount != newCount) {
      if (newCount > 0) {
        m_audioEngine->playCountdownBeep();
      }
      else if (newCount == 0) {
        m_audioEngine->playCountdownStart();
      }
    }

    if (m_timer <= 0.0f) {
      m_isCountingDown = false;
      m_hasFinished = true;
      std::cout << "Countdown complete - calling completion callback" << std::endl;
      if (m_onCountdownComplete) {
        m_onCountdownComplete();
      }
    }
  }
}

void RaceCountdown::reset() {
  m_timer = 0.0f;
  m_isCountingDown = false;
  m_hasFinished = false;
}

void RaceCountdown::draw(JAGEngine::SpriteBatch& batch, const JAGEngine::Camera2D& camera) {
  if (!m_font || !m_font->getTextureID()) {
    std::cout << "Invalid font or texture!" << std::endl;
    return;
  }

  std::string text = getText();
  if (text.empty()) return;

  // Calculate center position in screen coordinates
  glm::vec2 screenCenter = camera.getPosition();

  // Draw text centered
  m_font->draw(batch,
    text.c_str(),
    screenCenter,
    glm::vec2(2.0f),
    0.0f,
    JAGEngine::ColorRGBA8(0, 0, 0, 0), // black
    JAGEngine::Justification::MIDDLE); // Center align

  //std::cout << "Drawing text '" << text << "' at position: "
  //  << screenCenter.x << ", " << screenCenter.y << std::endl;
}

std::string RaceCountdown::getText() const {
  if (!m_hasFinished && !m_isCountingDown) {
    return "Press 'Begin Race!'";
  }
  else if (m_isCountingDown) {
    int count = static_cast<int>(std::ceil(m_timer));
    if (count > 0) {
      return std::to_string(count);
    }
    else {
      return "START!";
    }
  }
  return "";
}


void RaceCountdown::setFont(std::unique_ptr<JAGEngine::SpriteFont> font) {
  m_font = std::move(font);
}
