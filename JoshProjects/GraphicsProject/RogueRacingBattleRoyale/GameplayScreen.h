
#pragma once

#include <JAGEngine/IGameScreen.h>
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

};

