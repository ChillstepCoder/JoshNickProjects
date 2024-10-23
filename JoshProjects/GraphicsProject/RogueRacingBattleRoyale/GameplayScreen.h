
#pragma once


#include <JAGEngine/IGameScreen.h>
#include <Box2D/box2d.h>
#include <iostream>

class GameplayScreen : public JAGEngine::IGameScreen {
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

  //std::unique_ptr<b2World> m_world;
};

