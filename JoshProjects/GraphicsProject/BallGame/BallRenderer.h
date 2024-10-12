#pragma once

#include <JAGEngine/SpriteBatch.h>
#include <JAGEngine/GLSLProgram.h>
#include <vector>
#include <memory>
#include "Ball.h"

// Ball renderer interface
class BallRenderer {
public:
    virtual void renderBalls(JAGEngine::SpriteBatch& spriteBatch, const std::vector<Ball>& balls,
                             const glm::mat4& projectionMatrix);
    void setHueShift(float hueShift) { m_hueShift = hueShift; }
protected:
    std::unique_ptr<JAGEngine::GLSLProgram> m_program = nullptr;
    float m_hueShift = 0.0f;

    JAGEngine::ColorRGBA8 applyHueShift(const JAGEngine::ColorRGBA8& color) const;
};

// Visualizes kinetic energy
class MomentumBallRenderer : public BallRenderer {
public:
    virtual void renderBalls(JAGEngine::SpriteBatch& spriteBatch, const std::vector<Ball>& balls,
                             const glm::mat4& projectionMatrix) override;
};

// Visualizes positive X component of velocity, as well as position
class VelocityBallRenderer : public BallRenderer {
public:
    VelocityBallRenderer(int screenWidth, int screenHeight);

    virtual void renderBalls(JAGEngine::SpriteBatch& spriteBatch, const std::vector<Ball>& balls,
                             const glm::mat4& projectionMatrix) override;
private:
    int m_screenWidth;
    int m_screenHeight;
};

// Trippy renderer!
class TrippyBallRenderer : public BallRenderer {
public:
    TrippyBallRenderer(int screenWidth, int screenHeight);

    virtual void renderBalls(JAGEngine::SpriteBatch& spriteBatch, const std::vector<Ball>& balls,
                             const glm::mat4& projectionMatrix) override;
private:
    int m_screenWidth;
    int m_screenHeight;
    float m_time = 0.0f;
};
