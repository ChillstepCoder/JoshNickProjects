//MainGame.cpp

#include "MainGame.h"
#include "Errors.h"
#include <JAGEngine/ResourceManager.h>
#include <iostream>
#include <string>

MainGame::MainGame() :
  _screenWidth(1024),
  _screenHeight(768),
  _time(0),
  _gameState(GameState::PLAY),
  _maxFPS(60.0f)
{
  _camera.init(_screenWidth, _screenHeight);
  _camera.setPosition(glm::vec2(_screenWidth / 2.0f, _screenHeight / 2.0f));
  _camera.setScale(1.0f);
}

  MainGame::~MainGame() {
    SDL_Quit();
  }

  void MainGame::run() {
    initSystems();
    // _playerTexture = ImageLoader::loadPNG("Textures/JimmyJump_Pack/PNG/HappyCLoud.png");
        
    gameLoop();
  }

  void MainGame::initSystems() {

    JAGEngine::init();
    glViewport(0, 0, _screenWidth, _screenHeight);
    _window.create("Game Engine", _screenWidth, _screenHeight, 0);

    // Do all of this AFTER creating the window / OpenGL context
    initShaders();
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(0.0f, 0.0f, 0.3f, 1.0f);  // Dark blue background

    _spriteBatch.init();
  }

  void MainGame::gameLoop() {
    while (_gameState != GameState::EXIT) {
      //frame time measuring
      float startTicks = SDL_GetTicks();

      processInput();
      _time += 0.01f;


      _camera.update();

      drawGame();
      calculateFPS();

      static int frameCounter = 0;
      frameCounter++;
      if (frameCounter == 10) {
        std::cout << std::to_string((int)_fps) << std::endl;
        frameCounter = 0;
      }

      float frameTicks = SDL_GetTicks() - startTicks;

      //limit fps
      if (1000.0F / _maxFPS > frameTicks) {
        SDL_Delay(1000.0F / _maxFPS - frameTicks);
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

    const float CAMERA_SPEED = 30.0f;
    const float SCALE_SPEED = 0.4f;

    float ADJUSTED_SPEED = CAMERA_SPEED / (_camera.getScale());
    float ADJUSTED_SCALE_SPEED = SCALE_SPEED * sqrt(_camera.getScale());

    while (SDL_PollEvent(&evnt)) {
      switch (evnt.type) {
      case SDL_QUIT:
        _gameState = GameState::EXIT;
        break;
      case SDL_MOUSEMOTION:
        //std::cout << evnt.motion.x << " " << evnt.motion.x << std::endl;
        break;
      case SDL_KEYDOWN:
        switch (evnt.key.keysym.sym) {
        case SDLK_w:
          _camera.setPosition(_camera.getPosition() + glm::vec2(0.0f, ADJUSTED_SPEED));
          break;
        case SDLK_s:
          _camera.setPosition(_camera.getPosition() + glm::vec2(0.0f, -ADJUSTED_SPEED));
          break;
        case SDLK_a:
          _camera.setPosition(_camera.getPosition() + glm::vec2(-ADJUSTED_SPEED, 0.0f));
          break;
        case SDLK_d:
          _camera.setPosition(_camera.getPosition() + glm::vec2(ADJUSTED_SPEED, 0.0f));
          break;
        case SDLK_q:
          _camera.setScale(_camera.getScale() + ADJUSTED_SCALE_SPEED);
          break;
        case SDLK_e:
          _camera.setScale(_camera.getScale() - ADJUSTED_SCALE_SPEED);
          break;
          }
          break; 
      }
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
    glUniform1f(timeLocation, _time);

    GLuint pLocation = _colorProgram.getUniformLocation("P");
    // TODO: Josh - camera matrix is broken
    glm::mat4 cameraMatrix = _camera.getCameraMatrix();

    glUniformMatrix4fv(pLocation, 1, GL_FALSE, &(cameraMatrix[0][0]));

    _spriteBatch.begin();
    glm::vec4 pos(0.0f, 0.0f, 50.0f, 50.0f);
    glm::vec4 uv(0.0f, 0.0f, 1.0f, 1.0f);

    JAGEngine::GLTexture texture = JAGEngine::ResourceManager::getTexture("Textures/jimmyjump_pack/PNG/HappyCloud.png");
    std::cout << "Texture ID: " << texture.id << std::endl;

    JAGEngine::Color color;
    color.r = 255;
    color.g = 255;
    color.b = 255;
    color.a = 255;

    _spriteBatch.draw(pos, uv, texture.id, 0.1f, color);
    _spriteBatch.draw(pos + glm::vec4(50, 0, 0, 0), uv, texture.id, 0.1f, color);


    _spriteBatch.end();
    _spriteBatch.renderBatch();

    glBindTexture(GL_TEXTURE_2D, 0);

    _colorProgram.unuse();

    _window.swapBuffer();
  }

  void MainGame::calculateFPS() {
    static const int NUM_SAMPLES = 10;
    static float frameTimes[NUM_SAMPLES];
    static int currentFrame = 0;

    static float prevTicks = SDL_GetTicks();
    float currentTicks;

    currentTicks = SDL_GetTicks();

    _frameTime = currentTicks - prevTicks;
    frameTimes[currentFrame % NUM_SAMPLES] = _frameTime;

    prevTicks = currentTicks;

    int count;

    currentFrame++;

    if (currentFrame < NUM_SAMPLES) {
      count = currentFrame;
    }
    else {
      count = NUM_SAMPLES;
    }

    float frameTimeAverage = 0;
    for (int i = 0; i < count; i++) {
      frameTimeAverage += frameTimes[i];
    }
    frameTimeAverage /= count;

    if (frameTimeAverage > 0) {
      _fps = 1000.0f / frameTimeAverage;
    }
    else {
      _fps = 60.0f;
    }


  }
