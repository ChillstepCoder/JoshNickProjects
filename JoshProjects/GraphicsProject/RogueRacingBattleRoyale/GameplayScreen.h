#pragma once
#include <JAGEngine/IGameScreen.h>
#include <JAGEngine/SpriteBatch.h>
#include <JAGEngine/GLSLProgram.h>
#include <JAGEngine/ResourceManager.h>
#include "PhysicsSystem.h"
#include <glm/glm.hpp>
#include <memory>

class GameplayScreen : public JAGEngine::IGameScreen {
public:
  GameplayScreen();
  ~GameplayScreen();

  virtual void build() override;
  virtual void destroy() override;
  virtual void onEntry() override;
  virtual void onExit() override;
  virtual void update() override;
  virtual void draw() override;

  virtual int getNextScreenIndex() const override;
  virtual int getPreviousScreenIndex() const override;

private:
  void checkInput();
  void createTrack();
  void initShaders();
  void checkGLError(const char* location);

  std::unique_ptr<PhysicsSystem> m_physicsSystem;
  std::vector<b2BodyId> m_trackBodies;
  b2BodyId m_playerCarBody;
  GLuint m_carTexture;

  JAGEngine::SpriteBatch m_spriteBatch;
  JAGEngine::GLSLProgram m_textureProgram;
  glm::mat4 m_projectionMatrix;
};
