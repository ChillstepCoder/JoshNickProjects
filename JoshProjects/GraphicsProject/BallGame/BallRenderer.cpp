//BallRenderer.cpp

#include "BallRenderer.h"

void BallRenderer::renderBalls(JAGEngine::SpriteBatch& spriteBatch, const std::vector<Ball>& balls,
                               const glm::mat4& projectionMatrix) {
    //glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    // Lazily initialize the program
    if (m_program == nullptr) {
        m_program = std::make_unique<JAGEngine::GLSLProgram>();
        m_program->compileShaders("Shaders/textureShading.vert", "Shaders/textureShading.frag");
        m_program->addAttribute("vertexPosition");
        m_program->addAttribute("vertexColor");
        m_program->addAttribute("vertexUV");
        m_program->linkShaders();
    }
    m_program->use();

    spriteBatch.begin();

    // Make sure the shader uses texture 0
    glActiveTexture(GL_TEXTURE0);
    GLint textureUniform = m_program->getUniformLocation("mySampler");
    glUniform1i(textureUniform, 0);

    // Grab the camera matrix
    GLint pUniform = m_program->getUniformLocation("P");
    glUniformMatrix4fv(pUniform, 1, GL_FALSE, &projectionMatrix[0][0]);

    // Render all the balls
    for (const auto& ball : balls) {
      const glm::vec4 destRect(ball.position.x - ball.radius, ball.position.y - ball.radius,
        ball.radius * 2.0f, ball.radius * 2.0f);
      const glm::vec4 uvRect(0.0f, 0.0f, 1.0f, 1.0f);

      // Apply hue shift to the ball's color
      JAGEngine::ColorRGBA8 shiftedColor = applyHueShift(ball.color);

      spriteBatch.draw(destRect, uvRect, ball.textureId, 0.0f, shiftedColor);
    }

    spriteBatch.end();
    spriteBatch.renderBatch();

    m_program->unuse();
}

void MomentumBallRenderer::renderBalls(JAGEngine::SpriteBatch& spriteBatch, const std::vector<Ball>& balls,
                               const glm::mat4& projectionMatrix) {

    glClearColor(0.0f, 0.1f, 0.5f, 1.0f);

    // Lazily initialize the program
    if (m_program == nullptr) {
        m_program = std::make_unique<JAGEngine::GLSLProgram>();
        m_program->compileShaders("Shaders/textureShading.vert", "Shaders/textureShading.frag");
        m_program->addAttribute("vertexPosition");
        m_program->addAttribute("vertexColor");
        m_program->addAttribute("vertexUV");
        m_program->linkShaders();
    }
    m_program->use();

    spriteBatch.begin();

    // Make sure the shader uses texture 0
    glActiveTexture(GL_TEXTURE0);
    GLint textureUniform = m_program->getUniformLocation("mySampler");
    glUniform1i(textureUniform, 0);

    // Grab the camera matrix
    GLint pUniform = m_program->getUniformLocation("P");
    glUniformMatrix4fv(pUniform, 1, GL_FALSE, &projectionMatrix[0][0]);

    // Render all the balls
    for (auto& ball : balls) {
        const glm::vec4 uvRect(0.0f, 0.0f, 1.0f, 1.0f);
        const glm::vec4 destRect(ball.position.x - ball.radius, ball.position.y - ball.radius,
                                 ball.radius * 2.0f, ball.radius * 2.0f);
        JAGEngine::ColorRGBA8 color;
        GLubyte colorVal = (GLubyte)(glm::clamp(glm::length(ball.velocity) * ball.mass * 12.0f, 0.0f, 255.0f));
        color.r = colorVal;
        color.g = colorVal;
        color.b = colorVal;
        color.a = colorVal;
        spriteBatch.draw(destRect, uvRect, ball.textureId, 0.0f, color);
    }

    spriteBatch.end();
    spriteBatch.renderBatch();

    m_program->unuse();
}

VelocityBallRenderer::VelocityBallRenderer(int screenWidth, int screenHeight) :
    m_screenWidth(screenWidth),
    m_screenHeight(screenHeight) {
    // Empty
}


void VelocityBallRenderer::renderBalls(JAGEngine::SpriteBatch& spriteBatch, const std::vector<Ball>& balls,
                                       const glm::mat4& projectionMatrix) {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // Lazily initialize the program
    if (m_program == nullptr) {
        m_program = std::make_unique<JAGEngine::GLSLProgram>();
        m_program->compileShaders("Shaders/textureShading.vert", "Shaders/textureShading.frag");
        m_program->addAttribute("vertexPosition");
        m_program->addAttribute("vertexColor");
        m_program->addAttribute("vertexUV");
        m_program->linkShaders();
    }
    m_program->use();

    spriteBatch.begin();

    // Make sure the shader uses texture 0
    glActiveTexture(GL_TEXTURE0);
    GLint textureUniform = m_program->getUniformLocation("mySampler");
    glUniform1i(textureUniform, 0);

    // Grab the camera matrix
    GLint pUniform = m_program->getUniformLocation("P");
    glUniformMatrix4fv(pUniform, 1, GL_FALSE, &projectionMatrix[0][0]);

    // Render all the balls
    for (auto& ball : balls) {
        const glm::vec4 uvRect(0.0f, 0.0f, 1.0f, 1.0f);
        const glm::vec4 destRect(ball.position.x - ball.radius, ball.position.y - ball.radius,
                                 ball.radius * 2.0f, ball.radius * 2.0f);
        JAGEngine::ColorRGBA8 color;

        float mult = 100.0f;
        GLubyte colorVal = (GLubyte)(glm::clamp(ball.velocity.x * mult, 0.0f, 255.0f));
        color.r = 128;
        color.g = (GLubyte)((ball.position.x / m_screenWidth) * 255.0f);
        color.b = (GLubyte)((ball.position.y / m_screenHeight) * 255.0f);
        color.a = colorVal;
        spriteBatch.draw(destRect, uvRect, ball.textureId, 0.0f, color);
    }

    spriteBatch.end();
    spriteBatch.renderBatch();

    m_program->unuse();
}

TrippyBallRenderer::TrippyBallRenderer(int screenWidth, int screenHeight) :
    m_screenWidth(screenWidth),
    m_screenHeight(screenHeight) {
    // Empty
}

void TrippyBallRenderer::renderBalls(JAGEngine::SpriteBatch& spriteBatch, const std::vector<Ball>& balls, const glm::mat4& projectionMatrix) {
    glClearColor(0.1f, 0.0f, 0.0f, 1.0f);

    // Lazily initialize the program
    if (m_program == nullptr) {
        m_program = std::make_unique<JAGEngine::GLSLProgram>();
        m_program->compileShaders("Shaders/textureShading.vert", "Shaders/textureShading.frag");
        m_program->addAttribute("vertexPosition");
        m_program->addAttribute("vertexColor");
        m_program->addAttribute("vertexUV");
        m_program->linkShaders();
    }
    m_program->use();

    spriteBatch.begin();

    // Make sure the shader uses texture 0
    glActiveTexture(GL_TEXTURE0);
    GLint textureUniform = m_program->getUniformLocation("mySampler");
    glUniform1i(textureUniform, 0);

    // Grab the camera matrix
    GLint pUniform = m_program->getUniformLocation("P");
    glUniformMatrix4fv(pUniform, 1, GL_FALSE, &projectionMatrix[0][0]);

    // Change these constants to get cool stuff
    float TIME_SPEED = 0.01f;
    float DIVISOR = 4.0f; // Increase to get more arms
    float SPIRAL_INTENSITY = 10.0f; // Increase to make it spiral more

    m_time += TIME_SPEED;

    // Render all the balls
    for (auto& ball : balls) {
        const glm::vec4 uvRect(0.0f, 0.0f, 1.0f, 1.0f);
        const glm::vec4 destRect(ball.position.x - ball.radius, ball.position.y - ball.radius,
                                 ball.radius * 2.0f, ball.radius * 2.0f);
        JAGEngine::ColorRGBA8 color;
       
        // Get vector from center point
        glm::vec2 centerVec = ball.position - glm::vec2(m_screenWidth / 2, m_screenHeight / 2);
        float centerDist = glm::length(centerVec);

        // Get angle from the horizontal
        float angle = atan2(centerVec.x, centerVec.y) / (3.1415926f / DIVISOR);
        // Move with time
        angle -= m_time;
        // Add the spiral
        angle += (centerDist / m_screenWidth) * SPIRAL_INTENSITY;

        color.r = (GLubyte)(angle * 255.0f);
        color.g = (GLubyte)(angle * 255.0f * cos(m_time));
        color.b = (GLubyte)(angle * 255.0f * sin(m_time));
        color.a = (GLubyte)(glm::clamp(1.0f - (centerDist / (m_screenWidth / 2.0f)), 0.0f, 1.0f) * 255.0f);
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
