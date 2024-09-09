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
#include "JAGEngine/SpriteBatch.h"
#include "JAGEngine/InputManager.h"
#include "JAGEngine/Timing.h"
#include "Bullet.h"
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

  JAGEngine::Window _window;
  int _screenWidth;
  int _screenHeight;
  GameState _gameState;

  JAGEngine::Sprite _sprite;
  JAGEngine::GLSLProgram _colorProgram;
  JAGEngine::Camera2D _camera;

  JAGEngine::SpriteBatch _spriteBatch;

  JAGEngine::InputManager _inputManager;
  JAGEngine::FpsLimiter _fpsLimiter;

  std::vector<Bullet> _bullets;

  float _fps;
  float _maxFPS;
  float _time;
};

