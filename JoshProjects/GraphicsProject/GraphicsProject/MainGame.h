//MainGame.h

#pragma once

#include <SDL/SDL.h>
#include <windows.h>
#include "JAGEngine/GLSLProgram.h"
#include "JAGEngine/Sprite.h"
#include <GL/glew.h> 
#include <stdio.h>
#include "JAGEngine/GLTexture.h"
#include "JAGEngine/Window.h"
#include "JAGEngine/JAGEngine.h"
#include "JAGEngine/Camera2D.h"
#include <vector>
#include <cmath>

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

  JAGEngine::Window _window;
  int _screenWidth;
  int _screenHeight;
  GameState _gameState;

  std::vector <JAGEngine::Sprite*> _sprites;
  JAGEngine::Sprite _sprite;
  JAGEngine::GLSLProgram _colorProgram;
  JAGEngine::Camera2D _camera;

  float _fps;
  float _maxFPS;
  float _frameTime;

  float _time;
};

