// GameplayScreen.h

#pragma once

#include <JAGEngine/SpriteBatch.h>
#include <JAGEngine/GLSLProgram.h>
#include <JAGEngine/ResourceManager.h>
#include <glm/glm.hpp>
#include <memory>
#include <JAGEngine/IGameScreen.h>
#include "PhysicsSystem.h"
#include "Car.h"
#include "InputState.h"
#include "ScreenState.h"
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
  void drawDebugWindow();
  void drawImGui();

  bool m_showMainMenu = true;
  bool m_showDebugWindow = true;
  std::unique_ptr<PhysicsSystem> m_physicsSystem;
  std::vector<b2BodyId> m_trackBodies;
  b2BodyId m_playerCarBody;
  GLuint m_carTexture;
  JAGEngine::SpriteBatch m_spriteBatch;
  JAGEngine::GLSLProgram m_textureProgram;
  glm::mat4 m_projectionMatrix;
  bool m_isExiting = false;
  bool m_imguiInitialized = false;
  std::unique_ptr<Car> m_car;
  Car::CarProperties m_defaultCarProps;
};
