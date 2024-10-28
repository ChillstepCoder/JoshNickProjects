// GameplayScreen.h

#pragma once

#include <JAGEngine/SpriteBatch.h>
#include <JAGEngine/GLSLProgram.h>
#include <JAGEngine/ResourceManager.h>

#include <glm/glm.hpp>
#include <memory>

#include <JAGEngine/IGameScreen.h>
#include "PhysicsSystem.h"
#include <iostream>
#include <memory>

#include <ImGui/imgui.h>
#include <ImGui/imgui_impl_sdl2.h>
#include <ImGui/imgui_impl_opengl3.h>

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
  void exitGame();
  void cleanupImGui();

  void drawImGui();  // Add this method
  bool m_showMainMenu = true;  // Control menu visibility

  std::unique_ptr<PhysicsSystem> m_physicsSystem;
  std::vector<b2BodyId> m_trackBodies;
  b2BodyId m_playerCarBody;
  GLuint m_carTexture;

  JAGEngine::SpriteBatch m_spriteBatch;
  JAGEngine::GLSLProgram m_textureProgram;
  glm::mat4 m_projectionMatrix;

  bool m_isExiting = false;
  bool m_imguiInitialized = false;
};
