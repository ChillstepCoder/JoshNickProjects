#pragma once

#include <Bengine/SpriteBatch.h>
#include <Bengine/GLSLProgram.h>
#include <vector>
#include <cmath>
#include <algorithm>
#include <memory>
#include "Ball.h"

// Ball renderer interface
class BallRenderer {
public:
    virtual void renderBalls(Bengine::SpriteBatch& spriteBatch, const std::vector<Ball>& balls,
        const glm::mat4& projectionMatrix, Bengine::GLSLProgram* optSharedShader, const glm::vec3& shaderColor);
protected:
    //std::unique_ptr<Bengine::GLSLProgram> m_program = nullptr;
};

// Visualizes kinetic energy
class MomentumBallRenderer : public BallRenderer {
public:
    virtual void renderBalls(Bengine::SpriteBatch& spriteBatch, const std::vector<Ball>& balls,
        const glm::mat4& projectionMatrix, Bengine::GLSLProgram* optSharedShader, const glm::vec3& shaderColor) override;
};

// Visualizes positive X component of velocity, as well as position
class VelocityBallRenderer : public BallRenderer {
public:
    VelocityBallRenderer(int screenWidth, int screenHeight);

    virtual void renderBalls(Bengine::SpriteBatch& spriteBatch, const std::vector<Ball>& balls,
        const glm::mat4& projectionMatrix, Bengine::GLSLProgram* optSharedShader, const glm::vec3& shaderColor) override;
private:
    int m_screenWidth;
    int m_screenHeight;
};

// Trippy renderer!
class TrippyBallRenderer : public BallRenderer {
public:
    TrippyBallRenderer(int screenWidth, int screenHeight);

    virtual void renderBalls(Bengine::SpriteBatch& spriteBatch, const std::vector<Ball>& balls,
        const glm::mat4& projectionMatrix, Bengine::GLSLProgram* optSharedShader, const glm::vec3& shaderColor) override;
private:
    int m_screenWidth;
    int m_screenHeight;
    float m_time = 0.0f;
};

// New renderer!
class NewBallRenderer : public BallRenderer {
public:
    NewBallRenderer(int screenWidth, int screenHeight);

    virtual void renderBalls(Bengine::SpriteBatch& spriteBatch, const std::vector<Ball>& balls,
        const glm::mat4& projectionMatrix, Bengine::GLSLProgram* optSharedShader, const glm::vec3& shaderColor) override;
private:
    int m_screenWidth;
    int m_screenHeight;
    float m_time = 0.0f;

    std::unique_ptr<Bengine::GLSLProgram> m_trippyFractalProgram;
};
