//MainGame.cpp

#include "MainGame.h"
#include <JAGEngine/JAGEngine.h>
#include "Errors.h"
#include <JAGEngine/ResourceManager.h>
#include <iostream>
#include <string>
#include "Level.h"
#include "Zombie.h"

MainGame::MainGame() :
  _screenWidth(1024),
  _screenHeight(768),
  _time(0),
  _gameState(GameState::PLAY),
  _maxFPS(60.0f),
  _player(nullptr)
{
  _camera.init(_screenWidth, _screenHeight);
  _camera.setPosition(glm::vec2(_screenWidth / 2.0f, _screenHeight / 2.0f));
  _camera.setScale(1.0f);
}

MainGame::~MainGame() {
  for (int i = 0; i < _levels.size(); i++) {
    delete _levels[i];
  }
  SDL_Quit();
}

void MainGame::run() {

  initSystems();
  //// _playerTexture = ImageLoader::loadPNG("Textures/JimmyJump_Pack/PNG/HappyCLoud.png");
  initLevel();

  gameLoop();
}

void MainGame::initSystems() {

  JAGEngine::init();
  glViewport(0, 0, _screenWidth, _screenHeight);
  _window.create("Zombie Game", _screenWidth, _screenHeight, 0);
  glClearColor(0.6f, 0.6f, 0.6f, 1.0f);
  initShaders();
  _agentSpriteBatch.init();
  //_spriteBatch.init();
  //_fpsLimiter.init(_maxFPS);
}

void MainGame::gameLoop() {
  while (_gameState == GameState::PLAY) {

    _fpsLimiter.begin();

    processInput();

    updateAgents();
    _time += 0.01f;

    _camera.setPosition(_player->getPosition());
    _camera.update();

    //for (int i = 0; i < _bullets.size();) {
    //  if (_bullets[i].update() == true) {
    //    _bullets[i] = _bullets.back();
    //    _bullets.pop_back();
    //  }
    //  else {
    //    i++;
    //  }
    //}

    drawGame();

    _fps = _fpsLimiter.end();

    static int frameCounter = 0;
    int frameReset = 60;

    frameCounter++;

    if (frameCounter == frameReset) { //print fps every fps frames
      std::cout << std::to_string((int)_fps) << std::endl;
      frameReset = (int)_fps;
      frameCounter = 0;
    }
  }
}

void MainGame::updateAgents() {
  //update all humans
  for (int i = 0; i < _humans.size(); i++) {
    _humans[i]->update(_levels[_currentLevel]->getLevelData(),
      _humans,
      _zombies);
  }
  //dont forget to update zombies
}

void MainGame::initShaders() {
  _textureProgram.compileShaders("Shaders/colorShading.vert", "Shaders/colorShading.frag");
  _textureProgram.addAttribute("vertexPosition");
  _textureProgram.addAttribute("vertexColor");
  _textureProgram.addAttribute("vertexUV");
  _textureProgram.linkShaders();
}

void MainGame::processInput() {
  SDL_Event evnt;
  const double MIN_ZOOM = 1e-6;
  const double MAX_ZOOM = 1e6;
  const double ZOOM_FACTOR = 1.08;
  const double BASE_MOVE_SPEED = 8.0;

  while (SDL_PollEvent(&evnt)) {
    switch (evnt.type) {
    case SDL_QUIT:
      _gameState = GameState::EXIT;
      break;
    case SDL_KEYDOWN:
      _inputManager.pressKey(evnt.key.keysym.sym);
      break;
    case SDL_KEYUP:
      _inputManager.releaseKey(evnt.key.keysym.sym);
      break;
    case SDL_MOUSEBUTTONDOWN:
      _inputManager.pressKey(evnt.button.button);
      break;
    case SDL_MOUSEBUTTONUP:
      _inputManager.releaseKey(evnt.button.button);
      break;
    case SDL_MOUSEMOTION:
      _inputManager.setMouseCoords(evnt.motion.x, evnt.motion.y);
      break;
    }
  }

  // Get current zoom level
  double currentZoom = static_cast<double>(_camera.getScale());

  // Handle zooming
  if (_inputManager.isKeyPressed(SDLK_q)) {
    currentZoom *= ZOOM_FACTOR;
    if (currentZoom > MAX_ZOOM) currentZoom = MAX_ZOOM;
  }
  if (_inputManager.isKeyPressed(SDLK_e)) {
    currentZoom /= ZOOM_FACTOR;
    if (currentZoom < MIN_ZOOM) currentZoom = MIN_ZOOM;
  }

  // Update camera scale
  _camera.setScale(static_cast<float>(currentZoom));

  // Calculate adjusted movement speed
  double adjustedSpeed = BASE_MOVE_SPEED / (currentZoom);

  // Handle movement
  glm::vec2 movement(0.0f, 0.0f);
  if (_inputManager.isKeyPressed(SDLK_w)) {
    movement.y += 1.0f;
  }
  if (_inputManager.isKeyPressed(SDLK_s)) {
    movement.y -= 1.0f;
  }
  if (_inputManager.isKeyPressed(SDLK_a)) {
    movement.x -= 1.0f;
  }
  if (_inputManager.isKeyPressed(SDLK_d)) {
    movement.x += 1.0f;
  }

  // Normalize diagonal movement
  if (movement.x != 0.0f && movement.y != 0.0f) {
    movement = glm::normalize(movement);
  }

  // Apply movement //make it only do this when not following the player?
 // _camera.setPosition(_camera.getPosition() + movement * static_cast<float>(adjustedSpeed));

  if (_inputManager.isKeyPressed(SDL_BUTTON_LEFT)) {
    glm::vec2 mouseCoords = _inputManager.getMouseCoords();
    mouseCoords = _camera.convertScreenToWorld(mouseCoords);

    glm::vec2 playerPosition(0.0f);
    glm::vec2 direction = mouseCoords - playerPosition;
    direction = glm::normalize(direction);
    //_bullets.emplace_back(playerPosition, direction, 200.00f, 1000);
  }
}

void MainGame::drawGame() {
  glClearDepth(1.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

 _textureProgram.use();

  glActiveTexture(GL_TEXTURE0);

  GLint textureUniform = _textureProgram.getUniformLocation("mySampler");
  glUniform1i(textureUniform, 0);

  GLuint timeLocation = _textureProgram.getUniformLocation("time");
  glUniform1f(timeLocation, _time);


 // // TODO: Josh - camera matrix is broken
  glm::mat4 projectionMatrix = _camera.getCameraMatrix();
  GLuint pUniform = _textureProgram.getUniformLocation("P");
  glUniformMatrix4fv(pUniform, 1, GL_FALSE, &(projectionMatrix[0][0]));

  //draw the level
  _levels[_currentLevel]->draw();

  //begin drawing agents
  _agentSpriteBatch.begin();

  // draw the humans
  for (int i = 0; i < _humans.size(); i++) {
    _humans[i]->draw(_agentSpriteBatch);
  }
  _agentSpriteBatch.end();
  _agentSpriteBatch.renderBatch();

  _textureProgram.unuse();

  _window.swapBuffer();
}

void MainGame::initLevel() {
  _levels.push_back(new Level("Levels/Level1.txt"));
  _currentLevel = 0;
  _player = new Player();
  _player->init(1.0f, _levels[_currentLevel]->getStartPlayerPos(),&_inputManager);

  _humans.push_back(_player);
}
