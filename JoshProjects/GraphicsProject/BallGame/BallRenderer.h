//BallRenderer.h

#pragma once
#include <JAGEngine/SpriteBatch.h>
#include <JAGEngine/GLSLProgram.h>
#include <vector>
#include <memory>
#include "Ball.h"

class BallRenderer {
public:
  virtual ~BallRenderer() = default;
  virtual void renderBalls(JAGEngine::SpriteBatch& spriteBatch, const std::vector<Ball>& balls,
    const glm::mat4& projectionMatrix);
  virtual void setHueShift(float hueShift) { m_hueShift = hueShift; }

protected:
  std::unique_ptr<JAGEngine::GLSLProgram> m_program = nullptr;
  float m_hueShift = 0.0f;
  JAGEngine::ColorRGBA8 applyHueShift(const JAGEngine::ColorRGBA8& color) const;
};

class MomentumBallRenderer : public BallRenderer {
public:
  void renderBalls(JAGEngine::SpriteBatch& spriteBatch, const std::vector<Ball>& balls,
    const glm::mat4& projectionMatrix) override;
};

class VelocityBallRenderer : public BallRenderer {
public:
  VelocityBallRenderer(int screenWidth, int screenHeight);
  void renderBalls(JAGEngine::SpriteBatch& spriteBatch, const std::vector<Ball>& balls,
    const glm::mat4& projectionMatrix) override;
private:
  int m_screenWidth;
  int m_screenHeight;
};

class TrippyBallRenderer : public BallRenderer {
public:
  TrippyBallRenderer(int screenWidth, int screenHeight);
  void renderBalls(JAGEngine::SpriteBatch& spriteBatch, const std::vector<Ball>& balls,
    const glm::mat4& projectionMatrix) override;
private:
  int m_screenWidth;
  int m_screenHeight;
  float m_time = 0.0f;
};

class PulsatingGlowBallRenderer : public BallRenderer {
public:
  PulsatingGlowBallRenderer(int screenWidth, int screenHeight);
  void renderBalls(JAGEngine::SpriteBatch& spriteBatch, const std::vector<Ball>& balls,
    const glm::mat4& projectionMatrix) override;

private:
  int m_screenWidth;
  int m_screenHeight;
  float m_time = 0.0f;
};

class RippleEffectBallRenderer : public BallRenderer {
public:
  RippleEffectBallRenderer(int screenWidth, int screenHeight);
  void renderBalls(JAGEngine::SpriteBatch& spriteBatch, const std::vector<Ball>& balls,
    const glm::mat4& projectionMatrix) override;

private:
  int m_screenWidth;
  int m_screenHeight;
  float m_time = 0.0f;
};

class EnergyVortexBallRenderer : public BallRenderer {
public:
  EnergyVortexBallRenderer(int screenWidth, int screenHeight);
  void renderBalls(JAGEngine::SpriteBatch& spriteBatch, const std::vector<Ball>& balls,
    const glm::mat4& projectionMatrix) override;

private:
  int m_screenWidth;
  int m_screenHeight;
  float m_time = 0.0f;
  std::vector<float> m_collisionIntensities;
};
