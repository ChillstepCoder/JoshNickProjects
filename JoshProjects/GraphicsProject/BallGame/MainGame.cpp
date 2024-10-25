//MainGame.cpp Ball Game

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

  // Create the window without specifying size, using SDL_WINDOW_FULLSCREEN_DESKTOP flag
  m_window.create("Ball Game", 0, 0, SDL_WINDOW_FULLSCREEN_DESKTOP);

  // Get the actual screen size
  SDL_GetWindowSize(m_window.getSDLWindow(), &m_screenWidth, &m_screenHeight);

  std::cout << "Actual screen size: " << m_screenWidth << "x" << m_screenHeight << std::endl;

  // Use the OpenGL context from the window
  SDL_GLContext glContext = m_window.getGLContext();
  if (glContext == nullptr) {
    std::cerr << "OpenGL context could not be retrieved!" << std::endl;
    // Handle error
  }

  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glViewport(0, 0, m_screenWidth, m_screenHeight);

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

  m_gravityStrength = 0.0f;
  m_gravityDirection = 270.0f;
  updateGravity();

}


void MainGame::initRenderers() {
  try {
    m_ballRenderers.push_back(std::make_unique<BallRenderer>());
    m_ballRenderers.push_back(std::make_unique<MomentumBallRenderer>());
    m_ballRenderers.push_back(std::make_unique<VelocityBallRenderer>(m_screenWidth, m_screenHeight));
    m_ballRenderers.push_back(std::make_unique<TrippyBallRenderer>(m_screenWidth, m_screenHeight));
    m_ballRenderers.push_back(std::make_unique<PulsatingGlowBallRenderer>(m_screenWidth, m_screenHeight));
    m_ballRenderers.push_back(std::make_unique<RippleEffectBallRenderer>(m_screenWidth, m_screenHeight));
    m_ballRenderers.push_back(std::make_unique<EnergyVortexBallRenderer>(m_screenWidth, m_screenHeight));
  }
  catch (const std::exception& e) {
    std::cerr << "Error initializing renderers: " << e.what() << std::endl;
  }
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
  float maxBallSize = std::max<float>(m_ballSizeRange.x, m_ballSizeRange.y);
  m_grid = std::make_unique<Grid>(m_screenWidth, m_screenHeight, maxBallSize);

  std::mt19937 randomEngine((unsigned int)time(nullptr));
  std::uniform_real_distribution<float> randX(0.0f, (float)m_screenWidth);
  std::uniform_real_distribution<float> randY(0.0f, (float)m_screenHeight);
  std::uniform_real_distribution<float> randSize(m_ballSizeRange.x, m_ballSizeRange.y);
  std::uniform_int_distribution<int> randColor(0, 255);

  m_balls.clear();
  m_balls.reserve(m_numBalls);

  for (int i = 0; i < m_numBalls; i++) {
    glm::vec2 pos(randX(randomEngine), randY(randomEngine));
    float radius = randSize(randomEngine);

    // Start with zero velocity
    glm::vec2 velocity(0.0f, 0.0f);

    JAGEngine::ColorRGBA8 color(randColor(randomEngine), randColor(randomEngine), randColor(randomEngine), 255);

    GLuint textureId = JAGEngine::ResourceManager::getTexture("Textures/circle.png").id;
    if (textureId == 0) {
      std::cerr << "Failed to load ball texture!" << std::endl;
    }

    // Use radius for mass calculation (assuming uniform density)
    float mass = radius * radius;

    m_balls.emplace_back(radius, mass, pos, velocity, textureId, color);
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


  // Let the BallRenderer add sprites to the batch
  m_ballRenderers[m_currentRenderer]->renderBalls(m_spriteBatch, m_balls, m_camera.getCameraMatrix());


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
  m_ballController.setMaxSpeed(m_maxBallSpeed);
  m_ballController.setSpeedMultiplier(m_ballSpeedMultiplier);
  m_ballController.setFriction(m_friction);
  m_ballController.updateBalls(m_balls, m_grid.get(), deltaTime, m_screenWidth, m_screenHeight);
  updateGravity();
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
          << ") World coords: (" << worldCoords.x << ", " << worldCoords.y
          << ") Screen size: " << m_screenWidth << "x" << m_screenHeight << std::endl;
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

  // Handle keyboard events
  const Uint8* keyState = SDL_GetKeyboardState(NULL);
  if (keyState[SDL_SCANCODE_1]) {
    m_currentRenderer++;
    if (m_currentRenderer >= (int)m_ballRenderers.size()) {
      m_currentRenderer = 0;
    }
    // Add a small delay to prevent multiple switches on a single press
    SDL_Delay(200);
  }
}

void MainGame::updateImGui() {
  ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
  ImGui::SetNextWindowSize(ImVec2(400, 450), ImGuiCond_FirstUseEver);

  ImGui::Begin("Ball Controls", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

  ImGui::Text("Shader Selection:");

  const char* shaderNames[] = {
      "Default", "Momentum", "Velocity", "Trippy", "Pulsating Glow", "Ripple Effect"
  };
  static int selectedShader = 0;

  // First row of shaders
  ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10, 0));
  for (int i = 0; i < 3; i++) {
    ImGui::RadioButton(shaderNames[i], &selectedShader, i);
    if (i < 2) ImGui::SameLine();
  }
  ImGui::PopStyleVar();

  ImGui::Dummy(ImVec2(0.0f, 5.0f)); // Add some vertical spacing

  // Second row of shaders
  ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10, 0));
  for (int i = 3; i < 6; i++) {
    ImGui::RadioButton(shaderNames[i], &selectedShader, i);
    if (i < 5) ImGui::SameLine();
  }
  ImGui::PopStyleVar();

  // Update the current renderer if the selection has changed
  if (selectedShader >= 0 && selectedShader < 6) {
    m_currentRenderer = selectedShader;
  }
  else {
    std::cerr << "Invalid shader selection: " << selectedShader << std::endl;
  }

  ImGui::Separator();

  // Configuration sliders
  ImGui::SliderInt("Number of Balls", &m_numBalls, 100, 10000);
  ImGui::SliderFloat2("Ball Size Range", &m_ballSizeRange.x, 1.0f, 20.0f, "%.1f");
  ImGui::SliderFloat("Hue Shift", &m_hueShift, 0.0f, 360.0f);

  ImGui::Text("Speed Controls:");

  if (ImGui::SliderFloat("Ball Speed Multiplier", &m_ballSpeedMultiplier, 0.1f, 10.0f, "%.2f")) {
    m_ballController.setSpeedMultiplier(m_ballSpeedMultiplier);
  }

  ImGui::SliderFloat("Max Ball Speed", &m_maxBallSpeed, 1.0f, 1000.0f);

  if (ImGui::SliderFloat("Friction", &m_friction, 0.0f, 1.0f, "%.3f")) {
    m_ballController.setFriction(m_friction);
  }

  ImGui::Separator();
  ImGui::Text("Gravity Controls:");

  if (ImGui::SliderFloat("Gravity Strength", &m_gravityStrength, 0.0f, 1000.0f, "%.1f")) {
    updateGravity();
  }

  if (ImGui::SliderFloat("Gravity Direction", &m_gravityDirection, 0.0f, 360.0f, "%.1f")) {
    m_gravityDirection = fmodf(m_gravityDirection, 360.0f);
    if (m_gravityDirection < 0) m_gravityDirection += 360.0f;
    updateGravity();
  }

  // Apply hue shift to all renderers that support it
  for (auto& renderer : m_ballRenderers) {
    if (auto* ballRenderer = dynamic_cast<BallRenderer*>(renderer.get())) {
      ballRenderer->setHueShift(m_hueShift);
    }
  }

  // Button to reinitialize the game
  if (ImGui::Button("Reinitialize Game")) {
    reinitializeGame();
  }

  ImGui::Text("ImGui Debug Info:");
  ImGui::Text("Window Position: (%.1f, %.1f)", ImGui::GetWindowPos().x, ImGui::GetWindowPos().y);
  ImGui::Text("Window Size: (%.1f, %.1f)", ImGui::GetWindowSize().x, ImGui::GetWindowSize().y);

  ImGui::End();
}

void MainGame::updateGravity() {
  // Convert to radians
  float radians = m_gravityDirection * (3.14159f / 180.0f);

  // Calculate gravity vector
  glm::vec2 gravityVec(
    m_gravityStrength * std::cos(radians),
    m_gravityStrength * std::sin(radians)  // Note: No negation here
  );

  m_ballController.setGravity(gravityVec);

  // Debug output
  std::cout << "Gravity Direction: " << m_gravityDirection
    << ", Gravity Vector: (" << gravityVec.x << ", " << gravityVec.y << ")" << std::endl;
}

void MainGame::reinitializeGame() {
  m_balls.clear();
  m_grid = std::make_unique<Grid>(m_screenWidth, m_screenHeight, CELL_SIZE);
  initBalls();
  m_ballController.setMaxSpeed(m_maxBallSpeed);
  m_ballController.setSpeedMultiplier(m_ballSpeedMultiplier);
  updateGravity();
}

