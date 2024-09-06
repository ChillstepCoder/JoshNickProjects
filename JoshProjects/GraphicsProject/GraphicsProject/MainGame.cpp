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
  _gameState(GameState::PLAY),
  _maxFPS(60.0f)
{

}
MainGame::~MainGame() {
  SDL_DestroyWindow(_window);
  SDL_Quit();
}

void MainGame::run() {
  initSystems();

  _sprites.push_back(new Sprite());
  _sprites.back()->init(-1.0f, -1.0f, 1.0f, 1.0f, "Textures/JimmyJump_Pack/PNG/HappyCLoud.png");
  _sprites.push_back(new Sprite());
  _sprites.back()->init(0.0f, -1.0f, 1.0f, 1.0f, "Textures/JimmyJump_Pack/PNG/HappyCLoud.png");
  _sprites.push_back(new Sprite());
  _sprites.back()->init(-1.0f, 0.0f, 1.0f, 1.0f, "Textures/JimmyJump_Pack/PNG/HappyCLoud.png");
  _sprites.push_back(new Sprite());
  _sprites.back()->init(0.0f, 0.0f, 1.0f, 1.0f, "Textures/JimmyJump_Pack/PNG/HappyCLoud.png");
  _sprites.push_back(new Sprite());
  _sprites.back()->init(-0.5f, -0.5f, 1.0f, 1.0f, "Textures/JimmyJump_Pack/PNG/HappyCLoud.png");


 // _playerTexture = ImageLoader::loadPNG("Textures/JimmyJump_Pack/PNG/HappyCLoud.png");

  gameLoop();
}

void MainGame::initSystems() {
  //initialize SDL
  SDL_Init(SDL_INIT_EVERYTHING);

  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

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
  //check opengl version
  std::printf("***   OpenGL Version: %s   ***", glGetString(GL_VERSION));

  glClearColor(0.0f, 0.0f, 1.0f, 1.0f);

  //set vsync
  SDL_GL_SetSwapInterval(1);

  initShaders();
}

void MainGame::gameLoop() {
  while (_gameState != GameState::EXIT) {
    //frame time measuring
    float startTicks = SDL_GetTicks();

    processInput();
    _time += 0.01f;
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

  while (SDL_PollEvent(&evnt)) {
    switch (evnt.type) {
    case SDL_QUIT:
      _gameState = GameState::EXIT;
      break;
    case SDL_MOUSEMOTION:
      //std::cout << evnt.motion.x << " " << evnt.motion.x << std::endl;
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

  for (int i = 0; i < _sprites.size(); i++) {
    _sprites[i]->draw();
  }

  glBindTexture(GL_TEXTURE_2D, 0);
  _colorProgram.unuse();

  SDL_GL_SwapWindow(_window);

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
  } else {
    _fps = 60.0f;
  }

  
}
