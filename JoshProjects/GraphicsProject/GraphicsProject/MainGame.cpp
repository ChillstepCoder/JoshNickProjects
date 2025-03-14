//MainGame.cpp

#include "MainGame.h"
#include "JAGEngine/JAGErrors.h"
#include <JAGEngine/ResourceManager.h>
#include <iostream>
#include <string>

MainGame::MainGame() :
  _screenWidth(1024),
  _screenHeight(768),
  m_time(0),
  _gameState(GameState::PLAY),
  m_maxFPS(60.0f)
{
  _camera.init(_screenWidth, _screenHeight);
  _camera.setPosition(glm::vec2(_screenWidth / 2.0f, _screenHeight / 2.0f));
  _camera.setScale(0.25f);
}

  MainGame::~MainGame() {
    SDL_Quit();
  }

  void MainGame::run() {
    initSystems();
    // m_playerTexture = ImageLoader::loadPNG("Textures/JimmyJump_Pack/PNG/HappyCLoud.png");
        
    gameLoop();
  }

  void MainGame::initSystems() {

    JAGEngine::init();
    glViewport(0, 0, _screenWidth, _screenHeight);
    _window.create("Game Engine", _screenWidth, _screenHeight, 0);

    // Do all of this AFTER creating the window / OpenGL context
    initShaders();
    
    m_spriteBatch.init();
    m_fpsLimiter.init(m_maxFPS);
  }

  void MainGame::gameLoop() {
    while (_gameState != GameState::EXIT) {

      m_fpsLimiter.begin();


      m_inputManager.update();
      processInput();
      m_time += 0.01f;


      _camera.update();

      for (int i = 0; i < m_bullets.size();) {
        if (m_bullets[i].update() == true) {
          m_bullets[i] = m_bullets.back();
          m_bullets.pop_back();
        } else {
          i++;
        }
      }

      drawGame();

      m_fps = m_fpsLimiter.end();

      static int frameCounter = 0;
      int frameReset = 60;
      
      frameCounter++;

      if (frameCounter == frameReset) { //print fps every fps frames
        std::cout << std::to_string((int)m_fps) << std::endl;
        frameReset = (int)m_fps;
        frameCounter = 0;
      }
    }
  }

  void MainGame::initShaders() {
    _colorProgram.compileShaders("Shaders/colorShading.vert", "Shaders/colorShading.frag");
    _colorProgram.addAttribute("vertexPosition");
    _colorProgram.addAttribute("vertexColor");
    _colorProgram.addAttribute("vertexUV");
    _colorProgram.linkShaders();
  }

  void MainGame::processInput() {
    SDL_Event evnt;
    const double MIN_ZOOM = 1e-10;
    const double MAX_ZOOM = 1e10;
    const double ZOOM_FACTOR = 1.001; 
    const double BASE_MOVE_SPEED = 8.0;

    while (SDL_PollEvent(&evnt)) {
      switch (evnt.type) {
      case SDL_QUIT:
        _gameState = GameState::EXIT;
        break;
      case SDL_KEYDOWN:
        m_inputManager.pressKey(evnt.key.keysym.sym);
        break;
      case SDL_KEYUP:
        m_inputManager.releaseKey(evnt.key.keysym.sym);
        break;
      case SDL_MOUSEBUTTONDOWN:
        m_inputManager.pressKey(evnt.button.button);
        break;
      case SDL_MOUSEBUTTONUP:
        m_inputManager.releaseKey(evnt.button.button);
        break;
      case SDL_MOUSEMOTION:
        m_inputManager.setMouseCoords(evnt.motion.x, evnt.motion.y);
        break;
      }
    }

    // Get current zoom level
    double currentZoom = static_cast<double>(_camera.getScale());

    // Handle zooming
    if (m_inputManager.isKeyPressed(SDLK_q)) {
      currentZoom *= ZOOM_FACTOR;
      if (currentZoom > MAX_ZOOM) currentZoom = MAX_ZOOM;
    }
    if (m_inputManager.isKeyPressed(SDLK_e)) {
      currentZoom /= ZOOM_FACTOR;
      if (currentZoom < MIN_ZOOM) currentZoom = MIN_ZOOM;
    }

    // Update camera scale
    _camera.setScale(static_cast<float>(currentZoom));

    // Calculate adjusted movement speed
    double adjustedSpeed = BASE_MOVE_SPEED / (currentZoom);

    // Handle movement
    glm::vec2 movement(0.0f, 0.0f);
    if (m_inputManager.isKeyPressed(SDLK_w)) {
      movement.y += 1.0f;
    }
    if (m_inputManager.isKeyPressed(SDLK_s)) {
      movement.y -= 1.0f;
    }
    if (m_inputManager.isKeyPressed(SDLK_a)) {
      movement.x -= 1.0f;
    }
    if (m_inputManager.isKeyPressed(SDLK_d)) {
      movement.x += 1.0f;
    }

    // Normalize diagonal movement
    if (movement.x != 0.0f && movement.y != 0.0f) {
      movement = glm::normalize(movement);
    }

    // Apply movement
    _camera.setPosition(_camera.getPosition() + movement * static_cast<float>(adjustedSpeed));

    if (m_inputManager.isKeyPressed(SDL_BUTTON_LEFT)) {
      glm::vec2 mouseCoords = m_inputManager.getMouseCoords();
      mouseCoords = _camera.convertScreenToWorld(mouseCoords);

      glm::vec2 playerPosition(0.0f);
      glm::vec2 direction = mouseCoords - playerPosition;
      direction = glm::normalize(direction);
      m_bullets.emplace_back(playerPosition, direction, 200.00f, 1000);
    }
  }

  void MainGame::drawGame() {
    glClearDepth(1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    _colorProgram.use();

    glActiveTexture(GL_TEXTURE0);

    GLint textureLocation = _colorProgram.getUniformLocation("mySampler");
    glUniform1i(textureLocation, 0);

    GLuint timeLocation = _colorProgram.getUniformLocation("time");
    glUniform1f(timeLocation, m_time);

    GLuint pLocation = _colorProgram.getUniformLocation("P");
    // TODO: Josh - camera matrix is broken
    glm::mat4 cameraMatrix = _camera.getCameraMatrix();

    glUniformMatrix4fv(pLocation, 1, GL_FALSE, &(cameraMatrix[0][0]));

    m_spriteBatch.begin();
    glm::vec4 pos(-500.0f, -500.0f, 1000.0f, 1000.0f);
    glm::vec4 uv(0.0f, 0.0f, 1.0f, 1.0f);

    JAGEngine::GLTexture texture = JAGEngine::ResourceManager::getTexture("Textures/jimmyjump_pack/PNG/HappyCloud.png");
    //std::cout << "Texture ID: " << texture.id << std::endl;

    JAGEngine::ColorRGBA8 color;
    color.r = 255;
    color.g = 255;
    color.b = 255;
    color.a = 255;

    m_spriteBatch.draw(pos, uv, texture.id, 0.1f, color);

    for (int i = 0; i < m_bullets.size(); i++) {
      m_bullets[i].draw(m_spriteBatch);
    }

    m_spriteBatch.end();
    m_spriteBatch.renderBatch();

    glBindTexture(GL_TEXTURE_2D, 0);

    _colorProgram.unuse();

    _window.swapBuffer();
  }
