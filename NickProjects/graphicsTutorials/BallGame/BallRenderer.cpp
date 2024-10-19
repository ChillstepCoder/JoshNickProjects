#include "BallRenderer.h"
#include <iostream>

void BallRenderer::renderBalls(Bengine::SpriteBatch& spriteBatch, const std::vector<Ball>& balls,
    const glm::mat4& projectionMatrix, Bengine::GLSLProgram* shaderProgram, const glm::vec3& shaderColor) {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    // Lazily initialize the program
    if (shaderProgram == nullptr) {
        std::cerr << "Error: Shader program is null in BallRenderer::renderBalls" << std::endl;
        return;
    }

    shaderProgram->use();

    spriteBatch.begin();

    // Make sure the shader uses texture 0
    glActiveTexture(GL_TEXTURE0);
    GLint textureUniform = shaderProgram->getUniformLocation("mySampler");
    glUniform1i(textureUniform, 0);

    // Grab the camera matrix
    GLint pUniform = shaderProgram->getUniformLocation("P");
    glUniformMatrix4fv(pUniform, 1, GL_FALSE, &projectionMatrix[0][0]);

    GLint colorUniform = shaderProgram->getUniformLocation("uColor");
    if (colorUniform != -1) {
        glUniform3f(colorUniform, shaderColor.r, shaderColor.g, shaderColor.b);
    }
    else {
        std::cerr << "Warning: uColor uniform not found in shader program." << std::endl;
    }

    // Render all the balls
    for (auto& ball : balls) {
        const glm::vec4 uvRect(0.0f, 0.0f, 1.0f, 1.0f);
        const glm::vec4 destRect(ball.position.x - ball.radius, ball.position.y - ball.radius,
            ball.radius * 2.0f, ball.radius * 2.0f);
        spriteBatch.draw(destRect, uvRect, ball.textureId, 0.0f, ball.color, 0.0f);
    }

    spriteBatch.end();
    spriteBatch.renderBatch();

    shaderProgram->unuse();
}

void MomentumBallRenderer::renderBalls(Bengine::SpriteBatch& spriteBatch, const std::vector<Ball>& balls,
    const glm::mat4& projectionMatrix, Bengine::GLSLProgram* shaderProgram, const glm::vec3& shaderColor) {

    glClearColor(0.0f, 0.1f, 0.5f, 1.0f);

    // Lazily initialize the program
    if (shaderProgram == nullptr) {
        shaderProgram->compileShaders("Shaders/textureShading.vert", "Shaders/textureShading.frag");
        shaderProgram->addAttribute("vertexPosition");
        shaderProgram->addAttribute("vertexColor");
        shaderProgram->addAttribute("vertexUV");
        shaderProgram->linkShaders();
    }
    shaderProgram->use();

    spriteBatch.begin();

    // Make sure the shader uses texture 0
    glActiveTexture(GL_TEXTURE0);
    GLint textureUniform = shaderProgram->getUniformLocation("mySampler");
    glUniform1i(textureUniform, 0);

    // Grab the camera matrix
    GLint pUniform = shaderProgram->getUniformLocation("P");
    glUniformMatrix4fv(pUniform, 1, GL_FALSE, &projectionMatrix[0][0]);

    // Render all the balls
    for (auto& ball : balls) {
        const glm::vec4 uvRect(0.0f, 0.0f, 1.0f, 1.0f);
        const glm::vec4 destRect(ball.position.x - ball.radius, ball.position.y - ball.radius,
            ball.radius * 2.0f, ball.radius * 2.0f);
        Bengine::ColorRGBA8 color;
        GLubyte colorVal = (GLubyte)(glm::clamp(glm::length(ball.velocity) * ball.mass * 12.0f, 0.0f, 255.0f));
        color.r = colorVal;
        color.g = colorVal;
        color.b = colorVal;
        color.a = colorVal;
        spriteBatch.draw(destRect, uvRect, ball.textureId, 0.0f, color, 0.0f);
    }

    spriteBatch.end();
    spriteBatch.renderBatch();

    shaderProgram->unuse();
}

VelocityBallRenderer::VelocityBallRenderer(int screenWidth, int screenHeight) :
    m_screenWidth(screenWidth),
    m_screenHeight(screenHeight) {
    // Empty
}


void VelocityBallRenderer::renderBalls(Bengine::SpriteBatch& spriteBatch, const std::vector<Ball>& balls,
    const glm::mat4& projectionMatrix, Bengine::GLSLProgram* shaderProgram, const glm::vec3& shaderColor) {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // Lazily initialize the program
    if (shaderProgram == nullptr) {
        shaderProgram->compileShaders("Shaders/textureShading.vert", "Shaders/textureShading.frag");
        shaderProgram->addAttribute("vertexPosition");
        shaderProgram->addAttribute("vertexColor");
        shaderProgram->addAttribute("vertexUV");
        shaderProgram->linkShaders();
    }
    shaderProgram->use();

    spriteBatch.begin();

    // Make sure the shader uses texture 0
    glActiveTexture(GL_TEXTURE0);
    GLint textureUniform = shaderProgram->getUniformLocation("mySampler");
    glUniform1i(textureUniform, 0);

    // Grab the camera matrix
    GLint pUniform = shaderProgram->getUniformLocation("P");
    glUniformMatrix4fv(pUniform, 1, GL_FALSE, &projectionMatrix[0][0]);

    // Render all the balls
    for (auto& ball : balls) {
        const glm::vec4 uvRect(0.0f, 0.0f, 1.0f, 1.0f);
        const glm::vec4 destRect(ball.position.x - ball.radius, ball.position.y - ball.radius,
            ball.radius * 2.0f, ball.radius * 2.0f);
        Bengine::ColorRGBA8 color;

        float mult = 100.0f;
        GLubyte colorVal = (GLubyte)(glm::clamp(ball.velocity.x * mult, 0.0f, 255.0f));
        color.r = 128;
        color.g = (GLubyte)((ball.position.x / m_screenWidth) * 255.0f);
        color.b = (GLubyte)((ball.position.y / m_screenHeight) * 255.0f);
        color.a = colorVal;
        spriteBatch.draw(destRect, uvRect, ball.textureId, 0.0f, color, 0.0f);
    }

    spriteBatch.end();
    spriteBatch.renderBatch();

    shaderProgram->unuse();
}

TrippyBallRenderer::TrippyBallRenderer(int screenWidth, int screenHeight) :
    m_screenWidth(screenWidth),
    m_screenHeight(screenHeight) {
    // Empty
}

void TrippyBallRenderer::renderBalls(Bengine::SpriteBatch& spriteBatch, const std::vector<Ball>& balls, const glm::mat4& projectionMatrix, Bengine::GLSLProgram* shaderProgram, const glm::vec3& shaderColor) {
    glClearColor(0.1f, 0.0f, 0.0f, 1.0f);

    // Lazily initialize the program
    if (shaderProgram == nullptr) {
        shaderProgram->compileShaders("Shaders/textureShading.vert", "Shaders/textureShading.frag");
        shaderProgram->addAttribute("vertexPosition");
        shaderProgram->addAttribute("vertexColor");
        shaderProgram->addAttribute("vertexUV");
        shaderProgram->linkShaders();
    }
    shaderProgram->use();

    spriteBatch.begin();

    // Make sure the shader uses texture 0
    glActiveTexture(GL_TEXTURE0);
    GLint textureUniform = shaderProgram->getUniformLocation("mySampler");
    glUniform1i(textureUniform, 0);

    // Grab the camera matrix
    GLint pUniform = shaderProgram->getUniformLocation("P");
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
        Bengine::ColorRGBA8 color;

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
        spriteBatch.draw(destRect, uvRect, ball.textureId, 0.0f, color, 0.0f);
    }

    spriteBatch.end();
    spriteBatch.renderBatch();

    shaderProgram->unuse();
}

NewBallRenderer::NewBallRenderer(int screenWidth, int screenHeight) :
    m_screenWidth(screenWidth),
    m_screenHeight(screenHeight) {
    // Empty
}



void NewBallRenderer::renderBalls(Bengine::SpriteBatch& spriteBatch, const std::vector<Ball>& balls, const glm::mat4& projectionMatrix, Bengine::GLSLProgram* shaderProgram, const glm::vec3& shaderColor) {
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

    // Lazily initialize the program
    if (shaderProgram == nullptr) {
        shaderProgram->compileShaders("Shaders/textureShading.vert", "Shaders/textureShading.frag");
        shaderProgram->addAttribute("vertexPosition");
        shaderProgram->addAttribute("vertexColor");
        shaderProgram->addAttribute("vertexUV");
        shaderProgram->linkShaders();
    }
    shaderProgram->use();

    spriteBatch.begin();

    // Make sure the shader uses texture 0
    glActiveTexture(GL_TEXTURE0);
    GLint textureUniform = shaderProgram->getUniformLocation("mySampler");
    glUniform1i(textureUniform, 0);

    // Grab the camera matrix
    GLint pUniform = shaderProgram->getUniformLocation("P");
    glUniformMatrix4fv(pUniform, 1, GL_FALSE, &projectionMatrix[0][0]);

    // Change these constants to get cool stuff
    float TIME_SPEED = 0.02f;
    float DIVISOR = 0.0f; // Increase to get more arms
    float SPIRAL_INTENSITY = 3.0f; // Increase to make it spiral more

    m_time += TIME_SPEED;

    // Render all the balls
    for (auto& ball : balls) {
        const glm::vec4 uvRect(0.0f, 0.0f, 1.0f, 1.0f);
        const glm::vec4 destRect(ball.position.x - ball.radius, ball.position.y - ball.radius,
            ball.radius * 2.0f, ball.radius * 2.0f);
        Bengine::ColorRGBA8 color;

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
        spriteBatch.draw(destRect, uvRect, ball.textureId, 0.0f, color, 0.0f);
    }

    spriteBatch.end();
    spriteBatch.renderBatch();

    shaderProgram->unuse();
}
