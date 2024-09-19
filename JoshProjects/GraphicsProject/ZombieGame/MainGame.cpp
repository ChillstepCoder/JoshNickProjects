//MainGame.cpp

#include "MainGame.h"
#include <JAGEngine/JAGEngine.h>
#include <JAGEngine/Errors.h>
#include <JAGEngine/ResourceManager.h>
#include <iostream>
#include <fstream>
#include <string>
#include "Level.h"
#include "Zombie.h"
#include "Gun.h"
#include <random>
#include <ctime>
#include <algorithm>

const float HUMAN_SPEED = 2.2f;
const float ZOMBIE_SPEED = 1.7f;
const float PLAYER_SPEED = 3.6f;

const float TURN_TIME = 60.0f;

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

}

void MainGame::gameLoop() {

  const float DESIRED_FPS = 60.0f;
  const int MAX_PHYSICS_STEPS = 6;

  const float MS_PER_SECOND = 1000.0f;
  const float DESIRED_FRAMETIME = MS_PER_SECOND / DESIRED_FPS;
  const float MAX_DELTA_TIME = 1.0f;

  float previousTicks = SDL_GetTicks();


  while (_gameState == GameState::PLAY) {

    _fpsLimiter.begin();

    float newTicks = SDL_GetTicks();
    float frameTime = newTicks - previousTicks;
    previousTicks = newTicks;
    float totalDeltaTime = frameTime / DESIRED_FRAMETIME;

    checkVictory();

    _inputManager.update();

    processInput();

    int i = 0;
    while (totalDeltaTime > 0.0f && i < MAX_PHYSICS_STEPS) {
      float deltaTime = std::min(totalDeltaTime, MAX_DELTA_TIME);

      updateAgents(deltaTime);
      updateBullets(deltaTime);

      totalDeltaTime -= deltaTime;
      i++;
    }

    _time += 0.01f;

    _camera.setPosition(_player->getPosition());
    _camera.update();

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

void MainGame::updateAgents(float deltaTime) {
  if (_levels.empty() || _levels[_currentLevel] == nullptr) {
    std::cerr << "Error: No valid level data available." << std::endl;
    _gameState = GameState::EXIT;
    return;
  }

  const std::vector<std::string>& levelData = _levels[_currentLevel]->getLevelData();

  // Update all humans
  for (int i = 0; i < _humans.size(); i++) {
    _humans[i]->update(levelData, _humans, _zombies, deltaTime);
  }

  // Update all zombies
  for (int i = 0; i < _zombies.size(); i++) {
    _zombies[i]->update(levelData, _humans, _zombies, deltaTime);
  }

  //zombie collision
  for (int i = 0; i < _zombies.size(); i++) {
    //collide with zombies
    for (int j = i + 1; j < _zombies.size(); j++) {
      _zombies[i]->collideWithAgent(_zombies[j]);
    }
    // Collide with humans
    for (int j = 1; j < _humans.size(); j++) {
      if (_zombies[i]->collideWithAgent(_humans[j])) {
        _humans[j]->incrementZombify(1.0f * deltaTime);
        if (_humans[j]->getZombify() >= TURN_TIME) {
          // Add the new zombie
          _zombies.push_back(new Zombie);
          _zombies.back()->init(ZOMBIE_SPEED, _humans[j]->getPosition());
          // Delete the human
          delete _humans[j];
          _humans[j] = _humans.back();
          _humans.pop_back();
        }
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

void MainGame::updateBullets(float deltaTime) {
  //collide with world
  for (int i = 0; i < _bullets.size();) {
    if (_bullets[i].update(_levels[_currentLevel]->getLevelData(), deltaTime)) {
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
  if (_inputManager.isKeyDown(SDLK_q)) {
    currentZoom *= ZOOM_FACTOR;
    if (currentZoom > MAX_ZOOM) currentZoom = MAX_ZOOM;
  }
  if (_inputManager.isKeyDown(SDLK_e)) {
    currentZoom /= ZOOM_FACTOR;
    if (currentZoom < MIN_ZOOM) currentZoom = MIN_ZOOM;
  }

  // Update camera scale
  _camera.setScale(static_cast<float>(currentZoom));

  // Calculate adjusted movement speed
  double adjustedSpeed = BASE_MOVE_SPEED / (currentZoom);
  float shift = 1.0f;

  if (_inputManager.isKeyDown(SDLK_LSHIFT) || _inputManager.isKeyDown(SDLK_RSHIFT)) {
    shift = 2.0f;
  }
  else {
    shift = 1.0f;
  }

  // Handle movement
  glm::vec2 movement(0.0f, 0.0f);

  if (_inputManager.isKeyDown(SDLK_w)) {
    movement.y += 1.0f * shift;
  }
  if (_inputManager.isKeyDown(SDLK_s)) {
    movement.y -= 1.0f * shift;
  }
  if (_inputManager.isKeyDown(SDLK_a)) {
    movement.x -= 1.0f * shift;
  }
  if (_inputManager.isKeyDown(SDLK_d)) {
    movement.x += 1.0f * shift;
  }

  // Normalize diagonal movement
  if (movement.x != 0.0f && movement.y != 0.0f) {
    movement = glm::normalize(movement);
  }

  // Apply movement //make it only do this when not following the player?
 // _camera.setPosition(_camera.getPosition() + movement * static_cast<float>(adjustedSpeed));

  if (_inputManager.isKeyDown(SDL_BUTTON_LEFT)) {
    glm::vec2 mouseCoords = _inputManager.getMouseCoords();
    mouseCoords = _camera.convertScreenToWorld(mouseCoords);

    glm::vec2 playerPosition(0.0f);
    glm::vec2 direction = mouseCoords - playerPosition;
    direction = glm::normalize(direction);
    //_bullets.emplace_back(playerPosition, direction, 200.00f, 1000);
  }
}

void MainGame::checkVictory() {
  if (_zombies.empty()) {
    std::cout << "All zombies eliminated. Checking for next level..." << std::endl;
    if (doesNextLevelExist()) {
      std::cout << "Next level exists. Proceeding to load..." << std::endl;
      try {
        loadNextLevel();
        std::cout << "Next level loaded successfully." << std::endl;
      }
      catch (const std::exception& e) {
        std::cerr << "Error loading next level: " << e.what() << std::endl;
        _gameState = GameState::EXIT;
      }
    }
    else {
      std::cout << "No more levels. Game completed." << std::endl;
      std::printf("*** You Win! ***\n You killed %d humans and %d zombies. There are %d/%d civilians remaining",
        _numHumansKilled, _numZombiesKilled, _humans.size() - 1, _levels[_currentLevel]->getNumHumans());
      _gameState = GameState::EXIT;
    }
  }
}

void MainGame::loadNextLevel() {
  _currentLevel++;
  std::cout << "Loading level " << _currentLevel + 1 << std::endl;

  // Clear current level data
  std::cout << "Clearing current level data..." << std::endl;

  // Clear humans (except player)
  for (int i = 1; i < _humans.size(); i++) {
    delete _humans[i];
  }
  _humans.erase(_humans.begin() + 1, _humans.end());

  // Clear zombies
  for (auto& zombie : _zombies) {
    delete zombie;
  }
  _zombies.clear();

  // Clear bullets
  _bullets.clear();

  // Clear levels
  for (auto& level : _levels) {
    delete level;
  }
  _levels.clear();

  // Reset player position
  if (_player) {
    //_player->setPosition(glm::vec2(0, 0)); // Set to a safe initial position
  }

  std::cout << "All game data cleared." << std::endl;

  try {
    initLevel();
  }
  catch (const std::exception& e) {
    std::cerr << "Error in loadNextLevel: " << e.what() << std::endl;
    _gameState = GameState::EXIT;
  }

  std::cout << "Next level loaded successfully." << std::endl;
}

bool MainGame::doesNextLevelExist() {
  std::string nextLevelFileName = "Levels/Level" + std::to_string(_currentLevel + 2) + ".txt";
  std::ifstream file(nextLevelFileName);
  return file.good();
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
  std::string levelFileName = "Levels/Level" + std::to_string(_currentLevel + 1) + ".txt";
  std::cout << "Loading level: " << levelFileName << std::endl;

  try {
    _levels.push_back(new Level(levelFileName));
    std::cout << "Level object created." << std::endl;

    int width = _levels[0]->getWidth();
    int height = _levels[0]->getHeight();
    std::cout << "Level loaded. Width: " << width << ", Height: " << height << std::endl;

    // Initialize player
    if (_player == nullptr) {
      _player = new Player();
    }
    _player->init(PLAYER_SPEED, _levels[0]->getStartPlayerPos(), &_inputManager, &_camera, &_bullets);
    std::cout << "Player initialized at position: " << _player->getPosition().x << ", " << _player->getPosition().y << std::endl;

    // Ensure player is in _humans vector
    if (_humans.empty()) {
      _humans.push_back(_player);
    }
    else {
      _humans[0] = _player;
    }

    // Initialize humans
    std::mt19937 randomEngine(time(nullptr));
    std::uniform_int_distribution<int> randX(2, width - 2);
    std::uniform_int_distribution<int> randY(2, height - 2);

    for (int i = 0; i < _levels[0]->getNumHumans(); i++) {
      _humans.push_back(new Human);
      glm::vec2 pos(randX(randomEngine) * TILE_WIDTH, randY(randomEngine) * TILE_WIDTH);
      _humans.back()->init(HUMAN_SPEED, pos);
    }
    std::cout << "Humans initialized. Total humans: " << _humans.size() << std::endl;

    // Initialize zombies
    const std::vector<glm::vec2>& zombiePositions = _levels[0]->getZombieStartPositions();
    for (const auto& pos : zombiePositions) {
      _zombies.push_back(new Zombie);
      _zombies.back()->init(ZOMBIE_SPEED, pos);
    }
    std::cout << "Zombies initialized. Total zombies: " << _zombies.size() << std::endl;

    // Set up player's guns
    _player->addGun(new Gun("Pistol", 20, 1, 0.15f, 40, 20.0f));
    _player->addGun(new Gun("Shotgun", 90, 20, 50.0f, 25, 25.0f));
    _player->addGun(new Gun("MP5", 3, 1, 0.5f, 25, 30.0f));
    std::cout << "Player's guns set up." << std::endl;

    std::cout << "Level initialization complete." << std::endl;
  }
  catch (const std::exception& e) {
    std::cerr << "Error in initLevel: " << e.what() << std::endl;
    throw;
  }
}
