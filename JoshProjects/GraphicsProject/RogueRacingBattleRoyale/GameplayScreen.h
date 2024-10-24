// GameplayScreen.h
#pragma once
#include <JAGEngine/IGameScreen.h>
#include "PhysicsSystem.h"
#include <iostream>
#include <memory>

class GameplayScreen : public JAGEngine::IGameScreen {
public:
  GameplayScreen();
  ~GameplayScreen();

  virtual int getNextScreenIndex() const override;
  virtual int getPreviousScreenIndex() const override;
  virtual void build() override;
  virtual void destroy() override;
  virtual void onEntry() override;
  virtual void onExit() override;
  virtual void update() override;
  virtual void draw() override;

private:
  void checkInput();
  void initPhysics();
  void createTrack();

  std::unique_ptr<PhysicsSystem> m_physicsSystem;
  std::vector<b2BodyId> m_trackBodies;
  b2BodyId m_playerCarBody;
};

