//BallRenderer.cpp

#include "BallRenderer.h"
#include <algorithm>
#include <cmath>

void BallRenderer::renderBalls(JAGEngine::SpriteBatch& spriteBatch, const std::vector<Ball>& balls,
  const glm::mat4& projectionMatrix) {

  // Begin the sprite batch
  spriteBatch.begin();

  for (const auto& ball : balls) {
    const glm::vec4 destRect(ball.position.x - ball.radius, ball.position.y - ball.radius,
      ball.radius * 2.0f, ball.radius * 2.0f);
    const glm::vec4 uvRect(0.0f, 0.0f, 1.0f, 1.0f);
    JAGEngine::ColorRGBA8 shiftedColor = applyHueShift(ball.color);
    spriteBatch.draw(destRect, uvRect, ball.textureId, 0.0f, shiftedColor);
  }


  // End and render the sprite batch
  spriteBatch.end();
  spriteBatch.renderBatch();
}

void MomentumBallRenderer::renderBalls(JAGEngine::SpriteBatch& spriteBatch, const std::vector<Ball>& balls,
  const glm::mat4& projectionMatrix) {

  if (m_program == nullptr) {
    m_program = std::make_unique<JAGEngine::GLSLProgram>();
    m_program->compileShaders("Shaders/textureShading.vert", "Shaders/textureShading.frag");
    m_program->addAttribute("vertexPosition");
    m_program->addAttribute("vertexColor");
    m_program->addAttribute("vertexUV");
    m_program->linkShaders();
  }

  m_program->use();

  GLint pUniform = m_program->getUniformLocation("P");
  glUniformMatrix4fv(pUniform, 1, GL_FALSE, &projectionMatrix[0][0]);

  GLint textureUniform = m_program->getUniformLocation("mySampler");
  glUniform1i(textureUniform, 0);

  spriteBatch.begin();

  for (const auto& ball : balls) {
    const glm::vec4 destRect(ball.position.x - ball.radius, ball.position.y - ball.radius,
      ball.radius * 2.0f, ball.radius * 2.0f);
    const glm::vec4 uvRect(0.0f, 0.0f, 1.0f, 1.0f);
    GLubyte colorVal = (GLubyte)(glm::clamp(glm::length(ball.velocity) * ball.mass * 12.0f, 0.0f, 255.0f));
    JAGEngine::ColorRGBA8 color(colorVal, colorVal, colorVal, colorVal);
    spriteBatch.draw(destRect, uvRect, ball.textureId, 0.0f, color);
  }

  spriteBatch.end();
  spriteBatch.renderBatch();

  m_program->unuse();
}

VelocityBallRenderer::VelocityBallRenderer(int screenWidth, int screenHeight)
  : m_screenWidth(screenWidth), m_screenHeight(screenHeight) {}

void VelocityBallRenderer::renderBalls(JAGEngine::SpriteBatch& spriteBatch, const std::vector<Ball>& balls,
  const glm::mat4& projectionMatrix) {

  if (m_program == nullptr) {
    m_program = std::make_unique<JAGEngine::GLSLProgram>();
    m_program->compileShaders("Shaders/textureShading.vert", "Shaders/textureShading.frag");
    m_program->addAttribute("vertexPosition");
    m_program->addAttribute("vertexColor");
    m_program->addAttribute("vertexUV");
    m_program->linkShaders();
  }

  m_program->use();

  GLint pUniform = m_program->getUniformLocation("P");
  glUniformMatrix4fv(pUniform, 1, GL_FALSE, &projectionMatrix[0][0]);

  GLint textureUniform = m_program->getUniformLocation("mySampler");
  glUniform1i(textureUniform, 0);

  spriteBatch.begin();

  for (const auto& ball : balls) {
    const glm::vec4 destRect(ball.position.x - ball.radius, ball.position.y - ball.radius,
      ball.radius * 2.0f, ball.radius * 2.0f);
    const glm::vec4 uvRect(0.0f, 0.0f, 1.0f, 1.0f);
    float mult = 100.0f;
    GLubyte colorVal = (GLubyte)(glm::clamp(ball.velocity.x * mult, 0.0f, 255.0f));
    JAGEngine::ColorRGBA8 color(128,
      (GLubyte)((ball.position.x / m_screenWidth) * 255.0f),
      (GLubyte)((ball.position.y / m_screenHeight) * 255.0f),
      colorVal);
    spriteBatch.draw(destRect, uvRect, ball.textureId, 0.0f, color);
  }

  spriteBatch.end();
  spriteBatch.renderBatch();

  m_program->unuse();
}

TrippyBallRenderer::TrippyBallRenderer(int screenWidth, int screenHeight)
  : m_screenWidth(screenWidth), m_screenHeight(screenHeight) {}

void TrippyBallRenderer::renderBalls(JAGEngine::SpriteBatch& spriteBatch, const std::vector<Ball>& balls,
  const glm::mat4& projectionMatrix) {
  if (m_program == nullptr) {
    m_program = std::make_unique<JAGEngine::GLSLProgram>();
    m_program->compileShaders("Shaders/textureShading.vert", "Shaders/textureShading.frag");
    m_program->addAttribute("vertexPosition");
    m_program->addAttribute("vertexColor");
    m_program->addAttribute("vertexUV");
    m_program->linkShaders();
  }

  m_program->use();

  GLint pUniform = m_program->getUniformLocation("P");
  glUniformMatrix4fv(pUniform, 1, GL_FALSE, &projectionMatrix[0][0]);

  GLint textureUniform = m_program->getUniformLocation("mySampler");
  glUniform1i(textureUniform, 0);

  spriteBatch.begin();

  float TIME_SPEED = 0.01f;
  float DIVISOR = 4.0f;
  float SPIRAL_INTENSITY = 10.0f;

  m_time += TIME_SPEED;

  for (const auto& ball : balls) {
    const glm::vec4 destRect(ball.position.x - ball.radius, ball.position.y - ball.radius,
      ball.radius * 2.0f, ball.radius * 2.0f);
    const glm::vec4 uvRect(0.0f, 0.0f, 1.0f, 1.0f);

    glm::vec2 centerVec = ball.position - glm::vec2(m_screenWidth / 2, m_screenHeight / 2);
    float centerDist = glm::length(centerVec);
    float angle = atan2(centerVec.x, centerVec.y) / (3.1415926f / DIVISOR);
    angle -= m_time;
    angle += (centerDist / m_screenWidth) * SPIRAL_INTENSITY;

    JAGEngine::ColorRGBA8 color((GLubyte)(angle * 255.0f),
      (GLubyte)(angle * 255.0f * cos(m_time)),
      (GLubyte)(angle * 255.0f * sin(m_time)),
      (GLubyte)(glm::clamp(1.0f - (centerDist / (m_screenWidth / 2.0f)), 0.0f, 1.0f) * 255.0f));
    spriteBatch.draw(destRect, uvRect, ball.textureId, 0.0f, color);
  }

  spriteBatch.end();
  spriteBatch.renderBatch();

  m_program->unuse();
}

JAGEngine::ColorRGBA8 BallRenderer::applyHueShift(const JAGEngine::ColorRGBA8& color) const {
  float r = color.r / 255.0f;
  float g = color.g / 255.0f;
  float b = color.b / 255.0f;

  float cmax = std::max({ r, g, b });
  float cmin = std::min({ r, g, b });
  float diff = cmax - cmin;

  float h = 0.0f;
  float s = (cmax == 0.0f) ? 0.0f : diff / cmax;
  float v = cmax;

  if (cmax == cmin) {
    h = 0.0f;
  }
  else if (cmax == r) {
    h = fmod((60 * ((g - b) / diff) + 360), 360.0f);
  }
  else if (cmax == g) {
    h = fmod((60 * ((b - r) / diff) + 120), 360.0f);
  }
  else if (cmax == b) {
    h = fmod((60 * ((r - g) / diff) + 240), 360.0f);
  }

  // Apply hue shift
  h = fmod(h + m_hueShift, 360.0f);

  // Convert back to RGB
  float c = v * s;
  float x = c * (1 - fabs(fmod(h / 60.0f, 2) - 1));
  float m = v - c;

  if (h >= 0 && h < 60) {
    r = c; g = x; b = 0;
  }
  else if (h >= 60 && h < 120) {
    r = x; g = c; b = 0;
  }
  else if (h >= 120 && h < 180) {
    r = 0; g = c; b = x;
  }
  else if (h >= 180 && h < 240) {
    r = 0; g = x; b = c;
  }
  else if (h >= 240 && h < 300) {
    r = x; g = 0; b = c;
  }
  else {
    r = c; g = 0; b = x;
  }

  return JAGEngine::ColorRGBA8(
    static_cast<GLubyte>((r + m) * 255),
    static_cast<GLubyte>((g + m) * 255),
    static_cast<GLubyte>((b + m) * 255),
    color.a
  );
}

PulsatingGlowBallRenderer::PulsatingGlowBallRenderer(int screenWidth, int screenHeight)
  : m_screenWidth(screenWidth), m_screenHeight(screenHeight) {}

void PulsatingGlowBallRenderer::renderBalls(JAGEngine::SpriteBatch& spriteBatch, const std::vector<Ball>& balls,
  const glm::mat4& projectionMatrix) {
  if (m_program == nullptr) {
    m_program = std::make_unique<JAGEngine::GLSLProgram>();
    m_program->compileShaders("Shaders/pulsatingGlow.vert", "Shaders/pulsatingGlow.frag");
    m_program->addAttribute("vertexPosition");
    m_program->addAttribute("vertexColor");
    m_program->addAttribute("vertexUV");
    m_program->linkShaders();
  }

  m_program->use();

  GLint pUniform = m_program->getUniformLocation("P");
  glUniformMatrix4fv(pUniform, 1, GL_FALSE, &projectionMatrix[0][0]);

  GLint timeUniform = m_program->getUniformLocation("time");
  glUniform1f(timeUniform, m_time);

  GLint screenDimensionsUniform = m_program->getUniformLocation("screenDimensions");
  glUniform2f(screenDimensionsUniform, (float)m_screenWidth, (float)m_screenHeight);

  GLint textureUniform = m_program->getUniformLocation("mySampler");
  glUniform1i(textureUniform, 0);

  spriteBatch.begin();

  for (const auto& ball : balls) {
    const glm::vec4 destRect(ball.position.x - ball.radius, ball.position.y - ball.radius,
      ball.radius * 2.0f, ball.radius * 2.0f);
    const glm::vec4 uvRect(0.0f, 0.0f, 1.0f, 1.0f);
    JAGEngine::ColorRGBA8 shiftedColor = applyHueShift(ball.color);
    spriteBatch.draw(destRect, uvRect, ball.textureId, 0.0f, shiftedColor);
  }

  spriteBatch.end();
  spriteBatch.renderBatch();

  m_program->unuse();

  m_time += 1.0f / 60.0f; // Assuming 60 FPS, adjust as needed
}

RippleEffectBallRenderer::RippleEffectBallRenderer(int screenWidth, int screenHeight)
  : m_screenWidth(screenWidth), m_screenHeight(screenHeight) {}

void RippleEffectBallRenderer::renderBalls(JAGEngine::SpriteBatch& spriteBatch, const std::vector<Ball>& balls,
  const glm::mat4& projectionMatrix) {
  if (m_program == nullptr) {
    m_program = std::make_unique<JAGEngine::GLSLProgram>();
    m_program->compileShaders("Shaders/rippleEffect.vert", "Shaders/rippleEffect.frag");
    m_program->addAttribute("vertexPosition");
    m_program->addAttribute("vertexColor");
    m_program->addAttribute("vertexUV");
    m_program->linkShaders();
  }

  m_program->use();

  GLint pUniform = m_program->getUniformLocation("P");
  glUniformMatrix4fv(pUniform, 1, GL_FALSE, &projectionMatrix[0][0]);

  GLint timeUniform = m_program->getUniformLocation("time");
  glUniform1f(timeUniform, m_time);

  GLint screenDimensionsUniform = m_program->getUniformLocation("screenDimensions");
  glUniform2f(screenDimensionsUniform, (float)m_screenWidth, (float)m_screenHeight);

  GLint textureUniform = m_program->getUniformLocation("mySampler");
  glUniform1i(textureUniform, 0);

  spriteBatch.begin();

  for (const auto& ball : balls) {
    const glm::vec4 destRect(ball.position.x - ball.radius, ball.position.y - ball.radius,
      ball.radius * 2.0f, ball.radius * 2.0f);
    const glm::vec4 uvRect(0.0f, 0.0f, 1.0f, 1.0f);
    JAGEngine::ColorRGBA8 shiftedColor = applyHueShift(ball.color);
    spriteBatch.draw(destRect, uvRect, ball.textureId, 0.0f, shiftedColor);
  }

  spriteBatch.end();
  spriteBatch.renderBatch();

  m_program->unuse();

  m_time += 1.0f / 60.0f; // Assuming 60 FPS, adjust as needed
}

EnergyVortexBallRenderer::EnergyVortexBallRenderer(int screenWidth, int screenHeight)
  : m_screenWidth(screenWidth), m_screenHeight(screenHeight) {}

void EnergyVortexBallRenderer::renderBalls(JAGEngine::SpriteBatch& spriteBatch, const std::vector<Ball>& balls,
  const glm::mat4& projectionMatrix) {
  if (m_program == nullptr) {
    m_program = std::make_unique<JAGEngine::GLSLProgram>();
    try {
      m_program->compileShaders("Shaders/energyVortex.vert", "Shaders/energyVortex.frag");
      m_program->addAttribute("vertexPosition");
      m_program->addAttribute("vertexColor");
      m_program->addAttribute("vertexUV");
      m_program->linkShaders();

      // Check if linking was successful
      GLint isLinked = 0;
      glGetProgramiv(m_program->getProgramID(), GL_LINK_STATUS, &isLinked);
      if (isLinked == GL_FALSE) {
        GLint maxLength = 0;
        glGetProgramiv(m_program->getProgramID(), GL_INFO_LOG_LENGTH, &maxLength);
        std::vector<GLchar> infoLog(maxLength);
        glGetProgramInfoLog(m_program->getProgramID(), maxLength, &maxLength, &infoLog[0]);
        std::cerr << "Shader linking failed: " << std::string(infoLog.begin(), infoLog.end()) << std::endl;
        return;
      }
    }
    catch (const std::exception& e) {
      std::cerr << "Failed to compile or link Energy Vortex shaders: " << e.what() << std::endl;
      return;
    }
  }

  m_program->use();

  GLint pUniform = m_program->getUniformLocation("P");
  if (pUniform == -1) {
    std::cerr << "Uniform 'P' not found in Energy Vortex shader!" << std::endl;
  }
  else {
    glUniformMatrix4fv(pUniform, 1, GL_FALSE, &projectionMatrix[0][0]);
  }

  GLint timeUniform = m_program->getUniformLocation("time");
  if (timeUniform == -1) {
    std::cerr << "Uniform 'time' not found in Energy Vortex shader!" << std::endl;
  }
  else {
    glUniform1f(timeUniform, m_time);
  }

  GLint screenDimensionsUniform = m_program->getUniformLocation("screenDimensions");
  if (screenDimensionsUniform == -1) {
    std::cerr << "Uniform 'screenDimensions' not found in Energy Vortex shader!" << std::endl;

    // Print all active uniforms
    GLint numUniforms = 0;
    glGetProgramiv(m_program->getProgramID(), GL_ACTIVE_UNIFORMS, &numUniforms);
    std::cout << "Number of active uniforms: " << numUniforms << std::endl;
    for (GLint i = 0; i < numUniforms; i++) {
      GLchar uniformName[256];
      GLint size;
      GLenum type;
      glGetActiveUniform(m_program->getProgramID(), i, sizeof(uniformName), NULL, &size, &type, uniformName);
      std::cout << "Uniform #" << i << ": " << uniformName << std::endl;
    }
  }
  else {
    glUniform2f(screenDimensionsUniform, static_cast<float>(m_screenWidth), static_cast<float>(m_screenHeight));
  }

  GLint textureUniform = m_program->getUniformLocation("mySampler");
  if (textureUniform == -1) {
    std::cerr << "Uniform 'mySampler' not found in Energy Vortex shader!" << std::endl;
  }
  else {
    glUniform1i(textureUniform, 0);
  }

  GLint velocityUniform = m_program->getUniformLocation("ballVelocity");
  if (velocityUniform == -1) {
    std::cerr << "Uniform 'ballVelocity' not found in Energy Vortex shader!" << std::endl;
  }

  GLint collisionUniform = m_program->getUniformLocation("recentCollisionIntensity");
  if (collisionUniform == -1) {
    std::cerr << "Uniform 'recentCollisionIntensity' not found in Energy Vortex shader!" << std::endl;
  }

  spriteBatch.begin();

  m_collisionIntensities.resize(balls.size(), 0.0f);

  for (size_t i = 0; i < balls.size(); i++) {
    const auto& ball = balls[i];
    const glm::vec4 destRect(ball.position.x - ball.radius, ball.position.y - ball.radius,
      ball.radius * 2.0f, ball.radius * 2.0f);
    const glm::vec4 uvRect(0.0f, 0.0f, 1.0f, 1.0f);
    JAGEngine::ColorRGBA8 shiftedColor = applyHueShift(ball.color);

    m_collisionIntensities[i] *= 0.95f;

    if (velocityUniform != -1) {
      glUniform2f(velocityUniform, ball.velocity.x, ball.velocity.y);
    }
    if (collisionUniform != -1) {
      glUniform1f(collisionUniform, m_collisionIntensities[i]);
    }

    spriteBatch.draw(destRect, uvRect, ball.textureId, 0.0f, shiftedColor);
  }

  spriteBatch.end();
  spriteBatch.renderBatch();

  m_program->unuse();

  m_time += 1.0f / 60.0f;
}
