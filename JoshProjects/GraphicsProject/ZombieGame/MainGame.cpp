//MainGame.cpp

#include "MainGame.h"
#include <JAGEngine/JAGEngine.h>
#include <JAGEngine/Errors.h>
#include <JAGEngine/ResourceManager.h>
#include <iostream>
#include <string>
#include "Level.h"
#include "Zombie.h"
#include "Gun.h"
#include <random>
#include <ctime>

const float HUMAN_SPEED = 2.2f;
const float ZOMBIE_SPEED = 1.7f;
const float PLAYER_SPEED = 3.6f;

MainGame::MainGame() :
  _screenWidth(1024),
  _screenHeight(768),
  _time(0),
  _gameState(GameState::PLAY),
  _maxFPS(60.0f),
  _player(nullptr),
  _numHumansKilled(0),
  _numZombiesKilled(0)
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

    checkVictory();

    processInput();

    updateAgents();

    updateBullets();

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
  //update all zombies
  for (int i = 0; i < _zombies.size(); i++) {
    _zombies[i]->update(_levels[_currentLevel]->getLevelData(),
      _humans,
      _zombies);
  }

  //zombie collision
  for (int i = 0; i < _zombies.size(); i++) {
    //collide with zombies
    for (int j = i + 1; j < _zombies.size(); j++) {
      _zombies[i]->collideWithAgent(_zombies[j]);
    }
    //collide with humans
    for (int j = 1; j < _humans.size(); j++) {
      if (_zombies[i]->collideWithAgent(_humans[j])) {
        //add the new zombie
        _zombies.push_back(new Zombie);
        _zombies.back()->init(ZOMBIE_SPEED, _humans[j]->getPosition());
        //delete the human
        delete _humans[j];
        _humans[j] = _humans.back();
        _humans.pop_back();
      }
    }

    //collide with the player
    if (_zombies[i]->collideWithAgent(_player)) {
      JAGEngine::fatalError("YOU LOSE");
    }
  }

  //human collision
  for (int i = 0; i < _humans.size(); i++) {
    for (int j = i + 1; j < _humans.size(); j++) {
      _humans[i]->collideWithAgent(_humans[j]);
    }
  }

  //dont forget to update zombies
}

void MainGame::updateBullets() {
  //collide with world
  for (int i = 0; i < _bullets.size();) {
    if (_bullets[i].update(_levels[_currentLevel]->getLevelData())) {
      _bullets[i] = _bullets.back();
      _bullets.pop_back();
    }
    else {
      i++;
    }
  }

  bool wasBulletRemoved;

  //collide with humans and zombies
  for (int i = 0; i < _bullets.size(); i++) {
    wasBulletRemoved = false;
    //loop zombies
    for (int j = 0; j < _zombies.size();) {
      //check collision
      if (_bullets[i].collideWithAgent(_zombies[j])) {

        //damage zombie and kill if out of health
        if (_zombies[j]->applyDamage(_bullets[i].getDamage())) {
          // if zombie dies remove him
          delete _zombies[j];
          _zombies[j] = _zombies.back();
          _zombies.pop_back();
          _numZombiesKilled++;
        }  else {
          j++;
        }

        //remove the bullet
        _bullets[i] = _bullets.back();
        _bullets.pop_back();
        wasBulletRemoved = true;
        break;
      } else {
        j++;
      }
    }
    //loop humans
    if (wasBulletRemoved == false) {
      for (int j = 1; j < _humans.size();) {
        //check collision
        if (_bullets[i].collideWithAgent(_humans[j])) {

          //damage zombie and kill if out of health
          if (_humans[j]->applyDamage(_bullets[i].getDamage())) {
            // if zombie dies remove him
            delete _humans[j];
            _humans[j] = _humans.back();
            _humans.pop_back();
            _numHumansKilled++;
          }
          else {
            j++;
          }

          //remove the bullet
          _bullets[i] = _bullets.back();
          _bullets.pop_back();
          wasBulletRemoved = true;
          break;
        }
        else {
          j++;
        }
      }
    }
  }
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
  const double ZOOM_FACTOR = 1.02;
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

void MainGame::checkVictory() {
  //TODO: support for multiple levels
  //if all zombies are dead we win
  if (_zombies.empty()) {
    std::printf("*** You Win! ***\n You killed %d humans and %d zombies. There are %d/%d civilians remaining",
      _numHumansKilled, _numZombiesKilled, _humans.size()-1, _levels[_currentLevel]->getNumHumans());
    JAGEngine::fatalError("");
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
  // draw the zombies
  for (int i = 0; i < _zombies.size(); i++) {
    _zombies[i]->draw(_agentSpriteBatch);
  }

  //draw the bullets
  for (int i = 0; i < _bullets.size(); i++) {
    _bullets[i].draw(_agentSpriteBatch);
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
  _player->init(PLAYER_SPEED, _levels[_currentLevel]->getStartPlayerPos(),&_inputManager, &_camera, &_bullets);

  _humans.push_back(_player);

  std::mt19937 randomEngine;
  randomEngine.seed(time(nullptr));
  std::uniform_int_distribution<int> randX(2, _levels[_currentLevel]->getWidth()-2);
  std::uniform_int_distribution<int> randY(2, _levels[_currentLevel]->getHeight()-2);

  //Add all the random humans
  for (int i = 0; i < _levels[_currentLevel]->getNumHumans(); i++) {
    _humans.push_back(new Human);
    glm::vec2 pos(randX(randomEngine) * TILE_WIDTH, randY(randomEngine) * TILE_WIDTH);
    _humans.back()->init(HUMAN_SPEED, pos);

  }

  //add the zombies
  const std::vector<glm::vec2>& zombiePositions = _levels[_currentLevel]->getZombieStartPositions();
  
  for (int i = 0; i < zombiePositions.size(); i++) {
    _zombies.push_back(new Zombie);
    _zombies.back()->init(ZOMBIE_SPEED, zombiePositions[i]);

  }

  //set up player's guns
  _player->addGun(new Gun("Pistol", 20, 1, 0.15f, 40, 20.0f));
  _player->addGun(new Gun("Shotgun", 90, 15, 40.0f, 20, 30.0f));
  _player->addGun(new Gun("MP5", 4, 1, 0.5f, 10, 40.0f));
}
