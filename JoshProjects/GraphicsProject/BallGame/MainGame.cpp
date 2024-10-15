//MainGame.cpp

#define _CRT_SECURE_NO_WARNINGS // To shut up the compiler about sprintf...
#include "MainGame.h"

#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_sdl2.h"
#include "ImGui/imgui_impl_opengl3.h"

#include <JAGEngine/JAGEngine.h>

#include <JAGEngine/ResourceManager.h>
#include <SDL/SDL.h>
#include <random>
#include <ctime>
#include <algorithm>
#include <cmath>
#include <iostream>

// Some helpful constants.
const float DESIRED_FPS = 60.0f; // FPS the game is designed to run at
const int MAX_PHYSICS_STEPS = 6; // Max number of physics steps per frame
const float MS_PER_SECOND = 1000; // Number of milliseconds in a second
const float DESIRED_FRAMETIME = MS_PER_SECOND / DESIRED_FPS; // The desired frame time per frame
const float MAX_DELTA_TIME = 1.0f; // Maximum size of deltaTime

MainGame::~MainGame() {
    // Empty
}

// MainGame.cpp

void MainGame::run() {
  init();
  initBalls();

  while (m_gameState == GameState::RUNNING) {
    Uint32 startTime = SDL_GetTicks();

    processInput();

    update(1.0f / 60.0f);  // Fixed time step

    // Start the ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    updateImGui();

    // Clear the screen
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Render game
    draw();

    // Render ImGui
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    m_window.swapBuffer();

    Uint32 frameTime = SDL_GetTicks() - startTime;
    if (frameTime < 16) {  // Cap at ~60 FPS
      SDL_Delay(16 - frameTime);
    }
  }

  // Cleanup
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplSDL2_Shutdown();
  ImGui::DestroyContext();
}

// MainGame.cpp

void MainGame::init() {
  JAGEngine::init();

  m_screenWidth = 1920;
  m_screenHeight = 1080;

  glViewport(0, 0, m_screenWidth, m_screenHeight);

  m_window.create("Ball Game", m_screenWidth, m_screenHeight, SDL_WINDOW_OPENGL);

  // Use the OpenGL context from the window
  SDL_GLContext glContext = m_window.getGLContext();
  if (glContext == nullptr) {
    std::cerr << "OpenGL context could not be retrieved!" << std::endl;
    // Handle error
  }

  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  m_camera.init(m_screenWidth, m_screenHeight);
  m_camera.setPosition(glm::vec2(m_screenWidth / 2.0f, m_screenHeight / 2.0f));

  m_spriteBatch.init();

  m_spriteFont = std::make_unique<JAGEngine::SpriteFont>("Fonts/data-unifon.ttf", 40);

  m_textureProgram.compileShaders("Shaders/textureShading.vert", "Shaders/textureShading.frag");
  m_textureProgram.addAttribute("vertexPosition");
  m_textureProgram.addAttribute("vertexColor");
  m_textureProgram.addAttribute("vertexUV");
  m_textureProgram.linkShaders();

  m_fpsLimiter.setMaxFPS(60.0f);

  // Initialize ImGui
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  //ImGuiIO& io = ImGui::GetIO(); (void)io;
  ImGui::StyleColorsDark();
  ImGui_ImplSDL2_InitForOpenGL(m_window.getSDLWindow(), glContext);
  ImGui_ImplOpenGL3_Init("#version 330");


  SDL_GL_SetSwapInterval(1);  // Enable VSync

  initRenderers();

  // Check for OpenGL errors
  GLenum error = glGetError();
  if (error != GL_NO_ERROR) {
    std::cerr << "OpenGL Error in init(): " << error << std::endl;
  }
}


void MainGame::initRenderers() {
    m_ballRenderers.push_back(std::make_unique<BallRenderer>());
    m_ballRenderers.push_back(std::make_unique<MomentumBallRenderer>());
    m_ballRenderers.push_back(std::make_unique<VelocityBallRenderer>(m_screenWidth, m_screenHeight));
    m_ballRenderers.push_back(std::make_unique<TrippyBallRenderer>(m_screenWidth, m_screenHeight));

}

struct BallSpawn {
    BallSpawn(const JAGEngine::ColorRGBA8& colr,
              float rad, float m, float minSpeed,
              float maxSpeed, float prob) :
              color(colr),
              radius(rad),
              mass(m),
              randSpeed(minSpeed, maxSpeed),
              probability(prob) {
        // Empty
    }
    JAGEngine::ColorRGBA8 color;
    float radius;
    float mass;
    float probability;
    std::uniform_real_distribution<float> randSpeed;
};

void MainGame::initBalls() {

  // Initialize the grid
  m_grid = std::make_unique<Grid>(m_screenWidth, m_screenHeight, CELL_SIZE);

#define ADD_BALL(p, ...) \
    totalProbability += p; \
    possibleBalls.emplace_back(__VA_ARGS__);

  // Number of balls to spawn
  const int NUM_BALLS = 20000;

  // Random engine stuff
  std::mt19937 randomEngine((unsigned int)time(nullptr));
  std::uniform_real_distribution<float> randX(0.0f, (float)m_screenWidth);
  std::uniform_real_distribution<float> randY(0.0f, (float)m_screenHeight);
  std::uniform_real_distribution<float> randDir(-1.0f, 1.0f);

  // Add all possible balls
  std::vector <BallSpawn> possibleBalls;
  float totalProbability = 0.0f;

  /// Random values for ball types
  std::uniform_real_distribution<float> r1(2.0f, 6.0f);
  std::uniform_int_distribution<int> r2(0, 255);

  // Adds the balls using a macro
  ADD_BALL(1.0f, JAGEngine::ColorRGBA8(255, 255, 255, 255),
    2.0f, 1.0f, 0.1f, 7.0f, totalProbability);
  ADD_BALL(1.0f, JAGEngine::ColorRGBA8(1, 254, 145, 255),
    2.0f, 2.0f, 0.1f, 3.0f, totalProbability);
  ADD_BALL(1.0f, JAGEngine::ColorRGBA8(177, 0, 254, 255),
    3.0f, 4.0f, 0.0f, 0.0f, totalProbability)
    ADD_BALL(1.0f, JAGEngine::ColorRGBA8(254, 0, 0, 255),
      3.0f, 4.0f, 0.0f, 0.0f, totalProbability);
  ADD_BALL(1.0f, JAGEngine::ColorRGBA8(0, 255, 255, 255),
    3.0f, 4.0f, 0.0f, 0.0f, totalProbability);
  ADD_BALL(1.0f, JAGEngine::ColorRGBA8(255, 255, 0, 255),
    3.0f, 4.0f, 0.0f, 0.0f, totalProbability);
  // Make a bunch of random ball types
  for (int i = 0; i < 10000; i++) {
    ADD_BALL(1.0f, JAGEngine::ColorRGBA8(r2(randomEngine), r2(randomEngine), r2(randomEngine), 255),
      r1(randomEngine), r1(randomEngine), 0.0f, 0.0f, totalProbability);
  }

  // Random probability for ball spawn
  std::uniform_real_distribution<float> spawn(0.0f, totalProbability);

  // Small optimization that sets the size of the internal array to prevent
  // extra allocations.
  m_balls.reserve(NUM_BALLS);

  // Set up ball to spawn with default value
  BallSpawn* ballToSpawn = &possibleBalls[0];
  for (int i = 0; i < NUM_BALLS; i++) {
    // Get the ball spawn roll
    float spawnVal = spawn(randomEngine);
    // Figure out which ball we picked
    for (size_t j = 0; j < possibleBalls.size(); j++) {
      if (spawnVal <= possibleBalls[j].probability) {
        ballToSpawn = &possibleBalls[j];
        break;
      }
    }

    // Get random starting position
    glm::vec2 pos(randX(randomEngine), randY(randomEngine));

    // Hacky way to get a random direction
    glm::vec2 direction(randDir(randomEngine), randDir(randomEngine));
    if (direction.x != 0.0f || direction.y != 0.0f) { // The chances of direction == 0 are astronomically low
      direction = glm::normalize(direction);
    }
    else {
      direction = glm::vec2(1.0f, 0.0f); // default direction
    }

    // Add ball
    GLuint textureId = JAGEngine::ResourceManager::getTexture("Textures/circle.png").id;
    if (textureId == 0) {
      std::cerr << "Failed to load ball texture!" << std::endl;
      // Handle error (e.g., use a default texture or exit)
    }

    m_balls.emplace_back(ballToSpawn->radius, ballToSpawn->mass, pos, direction * ballToSpawn->randSpeed(randomEngine),
      textureId,
      ballToSpawn->color);
    // Add the ball do the grid. IF YOU EVER CALL EMPLACE BACK AFTER INIT BALLS, m_grid will have DANGLING POINTERS!
    m_grid->addBall(&m_balls.back());
  }
}

void MainGame::draw() {
  // Set the viewport to cover the whole window
  glViewport(0, 0, m_screenWidth, m_screenHeight);

  // Use the shader program
  m_textureProgram.use();

  glActiveTexture(GL_TEXTURE0);

  // Use the new camera matrix
  glm::mat4 projectionMatrix = m_camera.getCameraMatrix();
  GLint pUniform = m_textureProgram.getUniformLocation("P");
  glUniformMatrix4fv(pUniform, 1, GL_FALSE, &projectionMatrix[0][0]);

  // Set the texture uniform
  GLint textureUniform = m_textureProgram.getUniformLocation("mySampler");
  if (textureUniform == -1) {
    std::cerr << "Failed to get uniform location for mySampler" << std::endl;
  }
  glUniform1i(textureUniform, 0);

  // Begin the sprite batch
  m_spriteBatch.begin();

  // Let the BallRenderer add sprites to the batch
  m_ballRenderers[m_currentRenderer]->renderBalls(m_spriteBatch, m_balls, m_camera.getCameraMatrix());

  // End and render the sprite batch
  m_spriteBatch.end();
  m_spriteBatch.renderBatch();

  // Unuse the shader program
  m_textureProgram.unuse();

  // Draw the HUD
  drawHud();

  // Check for OpenGL errors
  GLenum error = glGetError();
  if (error != GL_NO_ERROR) {
    std::cerr << "OpenGL Error in draw(): " << error << std::endl;
  }
}

void MainGame::update(float deltaTime) {
  m_ballController.updateBalls(m_balls, m_grid.get(), deltaTime, m_screenWidth, m_screenHeight);

  // Add some debug output
  static int frameCount = 0;
  if (frameCount % 60 == 0) {  // Print every 60 frames
    std::cout << "Ball 0 position: (" << m_balls[0].position.x << ", " << m_balls[0].position.y << ")" << std::endl;
  }
  frameCount++;

  m_camera.update();
}

void MainGame::drawHud() {
    const JAGEngine::ColorRGBA8 fontColor(255, 0, 0, 255);
    // Convert float to char *
    char buffer[64];
    sprintf(buffer, "%.1f", m_fps);

    m_spriteBatch.begin();
    m_spriteFont->draw(m_spriteBatch, buffer, glm::vec2(0.0f, m_screenHeight - 32.0f),
                       glm::vec2(1.0f), 0.0f, fontColor);
    m_spriteBatch.end();
    m_spriteBatch.renderBatch();
}

void MainGame::processInput() {
  // Update input manager
  m_inputManager.update();

  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    ImGui_ImplSDL2_ProcessEvent(&event);

    switch (event.type) {
    case SDL_QUIT:
      m_gameState = GameState::EXIT;
      break;
    case SDL_MOUSEMOTION:
      if (!ImGui::GetIO().WantCaptureMouse) {
        glm::vec2 screenCoords(event.motion.x, event.motion.y);
        glm::vec2 worldCoords = m_camera.convertScreenToWorld(screenCoords);
        std::cout << "Screen coords: (" << screenCoords.x << ", " << screenCoords.y
          << ") World coords: (" << worldCoords.x << ", " << worldCoords.y << ")" << std::endl;
        m_ballController.onMouseMove(m_balls, worldCoords.x, worldCoords.y);
        m_inputManager.setMouseCoords(worldCoords.x, worldCoords.y);
      }
      break;
    case SDL_MOUSEBUTTONDOWN:
      if (!ImGui::GetIO().WantCaptureMouse) {
        glm::vec2 mouseCoords = m_camera.convertScreenToWorld(
          glm::vec2((float)event.button.x, (float)event.button.y));
        m_ballController.onMouseDown(m_balls, mouseCoords.x, mouseCoords.y);
        m_inputManager.pressKey(event.button.button);
      }
      break;
    case SDL_MOUSEBUTTONUP:
      if (!ImGui::GetIO().WantCaptureMouse) {
        m_ballController.onMouseUp(m_balls);
        m_inputManager.releaseKey(event.button.button);
      }
      break;
    }
  }

  if (m_inputManager.isKeyPressed(SDLK_ESCAPE)) {
    m_gameState = GameState::EXIT;
  }
  // Handle gravity changes
  if (m_inputManager.isKeyPressed(SDLK_LEFT)) {
    m_ballController.setGravityDirection(GravityDirection::LEFT);
  }
  else if (m_inputManager.isKeyPressed(SDLK_RIGHT)) {
    m_ballController.setGravityDirection(GravityDirection::RIGHT);
  }
  else if (m_inputManager.isKeyPressed(SDLK_UP)) {
    m_ballController.setGravityDirection(GravityDirection::UP);
  }
  else if (m_inputManager.isKeyPressed(SDLK_DOWN)) {
    m_ballController.setGravityDirection(GravityDirection::DOWN);
  }
  else if (m_inputManager.isKeyPressed(SDLK_SPACE)) {
    m_ballController.setGravityDirection(GravityDirection::NONE);
  }

  // Switch renderers
  if (m_inputManager.isKeyPressed(SDLK_1)) {
    m_currentRenderer++;
    if (m_currentRenderer >= (int)m_ballRenderers.size()) {
      m_currentRenderer = 0;
    }
  }
}

void MainGame::updateImGui() {
  ImGui::SetNextWindowPos(ImVec2(m_screenWidth / 2, m_screenHeight / 2), ImGuiCond_FirstUseEver, ImVec2(0.5f, 0.5f));
  ImGui::SetNextWindowSize(ImVec2(300, 200), ImGuiCond_FirstUseEver);

  ImGui::Begin("Ball Controls", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

  ImGui::Text("ImGui Debug Info:");
  ImGui::Text("Window Position: (%.1f, %.1f)", ImGui::GetWindowPos().x, ImGui::GetWindowPos().y);
  ImGui::Text("Window Size: (%.1f, %.1f)", ImGui::GetWindowSize().x, ImGui::GetWindowSize().y);

  ImGui::Separator();

  ImGui::SliderFloat("Hue Shift", &m_hueShift, 0.0f, 360.0f);

  if (ImGui::Button("Test Button")) {
    // Add some action here if needed
  }

  ImGui::End();
}
