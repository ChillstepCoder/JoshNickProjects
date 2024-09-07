//MainGame.cpp

#include "MainGame.h"
#include "Errors.h"
#include <iostream>
#include <string>

MainGame::MainGame() :
  _screenWidth(1024),
  _screenHeight(768),
  _time(0),
  _window(nullptr),
  _gameState(GameState::PLAY)
{

}
MainGame::~MainGame() {
  SDL_DestroyWindow(_window);
  SDL_Quit();
}

void MainGame::run() {
  initSystems();
  _sprite.init(-1.0f, -1.0f, 2.0f, 2.0f);


  gameLoop();
}

void MainGame::initSystems() {
  //initialize SDL
  SDL_Init(SDL_INIT_EVERYTHING);

  _window = SDL_CreateWindow("Game Engine", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, _screenWidth, _screenHeight, SDL_WINDOW_OPENGL);

  if (_window == nullptr) {
    fatalError("SDL Window could not be created!");
  }

  SDL_GLContext glContext = SDL_GL_CreateContext(_window);
  if (glContext == nullptr) {
    fatalError("SDL_GL context could not be created.");
  }

  GLenum error = glewInit();
  if (error != GLEW_OK) {
    fatalError("Could not initialize glew.");
  }

  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

  glClearColor(0.0f, 0.0f, 1.0f, 1.0f);

  initShaders();
}

void MainGame::gameLoop() {
  while (_gameState != GameState::EXIT) {
    processInput();
    _time += 0.01f;
    drawGame();
  }
}

void MainGame::initShaders() {
  _colorProgram.compileShaders("Shaders/colorShading.vert", "Shaders/colorShading.frag");
  _colorProgram.addAttribute("vertexPosition");
  _colorProgram.addAttribute("vertexColor");
  _colorProgram.linkShaders();
}

void MainGame::processInput() {

  SDL_Event evnt;

  while (SDL_PollEvent(&evnt)) {
    switch (evnt.type) {
    case SDL_QUIT:
      _gameState = GameState::EXIT;
      break;
    case SDL_MOUSEMOTION:
      std::cout << evnt.motion.x << " " << evnt.motion.x << std::endl;
      break;
    }
  }

}

void MainGame::drawGame() {

  glClearDepth(1.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  _colorProgram.use();

  GLuint timeLocation = _colorProgram.getUniformLocation("time");
  glUniform1f(timeLocation, _time);

  _sprite.draw();

  _colorProgram.unuse();

  SDL_GL_SwapWindow(_window);

}
