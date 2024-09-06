//MainGame.h

#pragma once

#include <SDL/SDL.h>
#include <windows.h>
#include "GLSLProgram.h"
#include "Sprite.h"
#include <GL/glew.h>
#include <stdio.h>
#include "GLTexture.h"
#include <vector>

enum class GameState {PLAY, EXIT};

class MainGame
{
public:
  MainGame();
  ~MainGame();

  void run();
private:
  void initSystems();
  void initShaders();
  void gameLoop();
  void processInput();
  void drawGame();
  void calculateFPS();

  SDL_Window* _window;
  int _screenWidth;
  int _screenHeight;
  GameState _gameState;

  std::vector <Sprite*> _sprites;
  Sprite _sprite;

  GLSLProgram _colorProgram;

  float _fps;
  float _maxFPS;
  float _frameTime;

  float _time;
};

