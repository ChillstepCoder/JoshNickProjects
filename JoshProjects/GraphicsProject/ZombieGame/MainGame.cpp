//MainGame.cpp

#include "MainGame.h"
#include <JAGEngine/JAGEngine.h>
#include <JAGEngine/JAGErrors.h>
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
#include "JAGEngine/Vertex.h"

#define GLM_ENABLE_EXPERIMENTAL

#include <glm/gtx/rotate_vector.hpp>

const float HUMAN_SPEED = 2.2f;
const float ZOMBIE_SPEED = 1.7f;
const float PLAYER_SPEED = 3.6f;

const float m_turnTime = 60.0f;

MainGame::MainGame() :
  m_screenWidth(1024),
  m_screenHeight(768),
  m_time(0),
  m_gameState(GameState::PLAY),
  m_maxFPS(60.0f),
  m_player(nullptr),
  m_numHumansKilled(0),
  m_numZombiesKilled(0)
{
  m_camera.init(m_screenWidth, m_screenHeight);
  m_camera.setPosition(glm::vec2(m_screenWidth / 2.0f, m_screenHeight / 2.0f));
  m_camera.setScale(1.0f);


  m_hudCamera.init(m_screenWidth, m_screenHeight);
  m_hudCamera.setPosition(glm::vec2(m_screenWidth / 2.0f, m_screenHeight / 2.0f));
  m_hudCamera.setScale(1.0f);
}

MainGame::~MainGame() {
  for (int i = 0; i < m_levels.size(); i++) {
    delete m_levels[i];
  }
  if (m_spriteFont) {
    m_spriteFont->dispose();
    delete m_spriteFont;
  }
  SDL_Quit();
}

void MainGame::run() {

  initSystems();

  initLevel();

  JAGEngine::Music music = m_audioEngine.loadMusic("Sound/bloodbath.ogg");
  music.play(-1);

  gameLoop();
}

void MainGame::initSystems() {
  std::cout << "Starting initSystems()" << std::endl;

  JAGEngine::init();
  std::cout << "JAGEngine initialized" << std::endl;

  //initialize sound
  m_audioEngine.init();

  glViewport(0, 0, m_screenWidth, m_screenHeight);
  std::cout << "Viewport set" << std::endl;

  m_window.create("Zombie Game", m_screenWidth, m_screenHeight, 0);
  std::cout << "Window created" << std::endl;

  glClearColor(0.6f, 0.6f, 0.6f, 1.0f);
  std::cout << "Clear color set" << std::endl;

  initShaders();
  std::cout << "Shaders initialized" << std::endl;

  m_agentSpriteBatch.init();
  std::cout << "Agent sprite batch initialized" << std::endl;

  m_hudSpriteBatch.init();
  std::cout << "HUD sprite batch initialized" << std::endl;

  m_spriteFont = new JAGEngine::SpriteFont();
  m_spriteFont->init("Fonts/titilium_bold.ttf", 64);

  m_bloodParticleBatch = new JAGEngine::ParticleBatch2D();
  m_bloodParticleBatch->init(1000, 0.075f,
    JAGEngine::ResourceManager::getTexture("Textures/zombie_game/particle.png"),
    [](JAGEngine::Particle2D particle, float deltaTime) {
      particle.position += particle.velocity * deltaTime;
      particle.color.a = (GLubyte)(particle.life * 255.0f);
  });
  m_particleEngine.addParticleBatch(m_bloodParticleBatch);

  std::cout << "initSystems() completed" << std::endl;
}

void MainGame::gameLoop() {

  const float DESIREDm_fps = 60.0f;
  const int MAX_PHYSICS_STEPS = 6;

  const float MS_PER_SECOND = 1000.0f;
  const float DESIRED_FRAMETIME = MS_PER_SECOND / DESIREDm_fps;
  const float MAX_DELTAm_time = 1.0f;

  float previousTicks = SDL_GetTicks();


  while (m_gameState == GameState::PLAY) {

    m_fpsLimiter.begin();

    float newTicks = SDL_GetTicks();
    float frameTime = newTicks - previousTicks;
    previousTicks = newTicks;
    float totalDeltaTime = frameTime / DESIRED_FRAMETIME;

    checkVictory();

    m_inputManager.update();

    processInput();

    int i = 0;
    while (totalDeltaTime > 0.0f && i < MAX_PHYSICS_STEPS) {
      float deltaTime = std::min(totalDeltaTime, MAX_DELTAm_time);

      updateAgents(deltaTime);
      updateBullets(deltaTime);
      m_particleEngine.update(deltaTime);

      totalDeltaTime -= deltaTime;
      i++;
    }

    m_time += 0.01f;

    m_camera.setPosition(m_player->getPosition());
    m_hudCamera.setPosition(m_player->getPosition());

    m_camera.update();
    m_hudCamera.update();

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

void MainGame::updateAgents(float deltaTime) {
  if (m_levels.empty() || m_levels[m_currentLevel] == nullptr) {
    std::cerr << "Error: No valid level data available." << std::endl;
    m_gameState = GameState::EXIT;
    return;
  }

  const std::vector<std::string>& levelData = m_levels[m_currentLevel]->getLevelData();

  // Update all humans
  for (int i = 0; i < m_humans.size(); i++) {
    m_humans[i]->update(levelData, m_humans, m_zombies, deltaTime);
  }

  // Update all zombies
  for (int i = 0; i < m_zombies.size(); i++) {
    m_zombies[i]->update(levelData, m_humans, m_zombies, deltaTime);
  }

  //zombie collision
  for (int i = 0; i < m_zombies.size(); i++) {
    //collide with zombies
    for (int j = i + 1; j < m_zombies.size(); j++) {
      m_zombies[i]->collideWithAgent(m_zombies[j]);
    }
    // Collide with humans
    for (int j = 1; j < m_humans.size(); j++) {
      if (m_zombies[i]->collideWithAgent(m_humans[j])) {
        //add blood
        addBlood((m_humans[j]->getPosition()+m_zombies[i]->getPosition())/2.0f, 1);
        m_humans[j]->incrementZombify(1.0f * deltaTime);
        if (m_humans[j]->getZombify() >= m_turnTime) {
          // Add the new zombie
          m_zombies.push_back(new Zombie);
          m_zombies.back()->init(ZOMBIE_SPEED, m_humans[j]->getPosition());
          // Delete the human
          delete m_humans[j];
          m_humans[j] = m_humans.back();
          m_humans.pop_back();
        }
      }
    }

    //collide with the player
    if (m_zombies[i]->collideWithAgent(m_player)) {
      JAGEngine::fatalError("YOU LOSE");
    }
  }

  //human collision
  for (int i = 0; i < m_humans.size(); i++) {
    for (int j = i + 1; j < m_humans.size(); j++) {
      m_humans[i]->collideWithAgent(m_humans[j]);
    }
  }

  //dont forget to update zombies
}

void MainGame::updateBullets(float deltaTime) {
  //collide with world
  for (int i = 0; i < m_bullets.size();) {
    if (m_bullets[i].update(m_levels[m_currentLevel]->getLevelData(), deltaTime)) {
      m_bullets[i] = m_bullets.back();
      m_bullets.pop_back();
    }
    else {
      i++;
    }
  }

  bool wasBulletRemoved;

  //collide with humans and zombies
  for (int i = 0; i < m_bullets.size(); i++) {
    wasBulletRemoved = false;
    //loop zombies
    for (int j = 0; j < m_zombies.size();) {
      //check collision
      if (m_bullets[i].collideWithAgent(m_zombies[j])) {
        //add blood
        addBlood(m_bullets[i].getPosition(), 5);

        //damage zombie and kill if out of health
        if (m_zombies[j]->applyDamage(m_bullets[i].getDamage())) {
          // if zombie dies remove him
          delete m_zombies[j];
          m_zombies[j] = m_zombies.back();
          m_zombies.pop_back();
          m_numZombiesKilled++;
        }  else {
          j++;
        }

        //remove the bullet
        m_bullets[i] = m_bullets.back();
        m_bullets.pop_back();
        wasBulletRemoved = true;
        break;
      } else {
        j++;
      }
    }
    //loop humans
    if (wasBulletRemoved == false) {
      for (int j = 1; j < m_humans.size();) {
        //check collision
        if (m_bullets[i].collideWithAgent(m_humans[j])) {
          addBlood(m_bullets[i].getPosition(), 5);

          //damage zombie and kill if out of health
          if (m_humans[j]->applyDamage(m_bullets[i].getDamage())) {
            // if zombie dies remove him
            delete m_humans[j];
            m_humans[j] = m_humans.back();
            m_humans.pop_back();
            m_numHumansKilled++;
          }
          else {
            j++;
          }

          //remove the bullet
          m_bullets[i] = m_bullets.back();
          m_bullets.pop_back();
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
  m_textureProgram.compileShaders("Shaders/colorShading.vert", "Shaders/colorShading.frag");
  m_textureProgram.addAttribute("vertexPosition");
  m_textureProgram.addAttribute("vertexColor");
  m_textureProgram.addAttribute("vertexUV");
  m_textureProgram.linkShaders();

  m_textRenderingProgram.compileShaders("Shaders/textRendering.vert", "Shaders/textRendering.frag");
  m_textRenderingProgram.addAttribute("vertexPosition");
  m_textRenderingProgram.addAttribute("vertexColor");
  m_textRenderingProgram.addAttribute("vertexUV");
  m_textRenderingProgram.linkShaders();
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
      m_gameState = GameState::EXIT;
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
  double currentZoom = static_cast<double>(m_camera.getScale());

  // Handle zooming
  if (m_inputManager.isKeyDown(SDLK_q)) {
    currentZoom *= ZOOM_FACTOR;
    if (currentZoom > MAX_ZOOM) currentZoom = MAX_ZOOM;
  }
  if (m_inputManager.isKeyDown(SDLK_e)) {
    currentZoom /= ZOOM_FACTOR;
    if (currentZoom < MIN_ZOOM) currentZoom = MIN_ZOOM;
  }

  // Update camera scale
  m_camera.setScale(static_cast<float>(currentZoom));

  // Calculate adjusted movement speed
  double adjustedSpeed = BASE_MOVE_SPEED / (currentZoom);
  float shift = 1.0f;

  if (m_inputManager.isKeyDown(SDLK_LSHIFT) || m_inputManager.isKeyDown(SDLK_RSHIFT)) {
    shift = 2.0f;
  }
  else {
    shift = 1.0f;
  }

  // Handle movement
  glm::vec2 movement(0.0f, 0.0f);

  if (m_inputManager.isKeyDown(SDLK_w)) {
    movement.y += 1.0f * shift;
  }
  if (m_inputManager.isKeyDown(SDLK_s)) {
    movement.y -= 1.0f * shift;
  }
  if (m_inputManager.isKeyDown(SDLK_a)) {
    movement.x -= 1.0f * shift;
  }
  if (m_inputManager.isKeyDown(SDLK_d)) {
    movement.x += 1.0f * shift;
  }

  // Normalize diagonal movement
  if (movement.x != 0.0f && movement.y != 0.0f) {
    movement = glm::normalize(movement);
  }

  // Apply movement //make it only do this when not following the player?
 // m_camera.setPosition(m_camera.getPosition() + movement * static_cast<float>(adjustedSpeed));

  if (m_inputManager.isKeyDown(SDL_BUTTON_LEFT)) {
    glm::vec2 mouseCoords = m_inputManager.getMouseCoords();
    mouseCoords = m_camera.convertScreenToWorld(mouseCoords);

    glm::vec2 playerPosition(0.0f);
    glm::vec2 direction = mouseCoords - playerPosition;
    direction = glm::normalize(direction);
    //m_bullets.emplace_back(playerPosition, direction, 200.00f, 1000);
  }
}

//unique pointers so i dont have to delete

void MainGame::checkVictory() {
  if (m_zombies.empty()) {
    std::cout << "All zombies eliminated. Checking for next level..." << std::endl;
    if (doesNextLevelExist()) {
      std::cout << "Next level exists. Proceeding to load..." << std::endl;
      try {
        loadNextLevel();
        std::cout << "Next level loaded successfully." << std::endl;
      }
      catch (const std::exception& e) {
        std::cerr << "Error loading next level: " << e.what() << std::endl;
        m_gameState = GameState::EXIT;
      }
    }
    else {
      std::cout << "No more levels. Game completed." << std::endl;
      std::printf("*** You Win! ***\n You killed %d humans and %d zombies. There are %d/%d civilians remaining",
        m_numHumansKilled, m_numZombiesKilled, m_humans.size() - 1, m_levels[m_currentLevel]->getNumHumans());
      m_gameState = GameState::EXIT;
    }
  }
}

void MainGame::loadNextLevel() {
  m_currentLevel++;
  std::cout << "Loading level " << m_currentLevel + 1 << std::endl;

  // Clear current level data
  std::cout << "Clearing current level data..." << std::endl;

  // Clear humans (except player)
  for (int i = 1; i < m_humans.size(); i++) {
    delete m_humans[i];
  }
  m_humans.erase(m_humans.begin() + 1, m_humans.end());

  // Clear zombies
  for (auto& zombie : m_zombies) {
    delete zombie;
  }
  m_zombies.clear();

  // Clear bullets
  m_bullets.clear();

  // Clear levels
  for (auto& level : m_levels) {
    delete level;
  }
  m_levels.clear();

  // Reset player position
  if (m_player) {
    //m_player->setPosition(glm::vec2(0, 0)); // Set to a safe initial position
  }

  std::cout << "All game data cleared." << std::endl;

  try {
    initLevel();
  }
  catch (const std::exception& e) {
    std::cerr << "Error in loadNextLevel: " << e.what() << std::endl;
    m_gameState = GameState::EXIT;
  }

  std::cout << "Next level loaded successfully." << std::endl;
}

bool MainGame::doesNextLevelExist() {
  std::string nextLevelFileName = "Levels/Level" + std::to_string(m_currentLevel + 2) + ".txt";
  std::ifstream file(nextLevelFileName);
  return file.good();
}

void MainGame::drawGame() {
  glClearDepth(1.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  m_textureProgram.use();

  glActiveTexture(GL_TEXTURE0);


  GLint textureUniform = m_textureProgram.getUniformLocation("mySampler");
  glUniform1i(textureUniform, 0);

  //make sure shader uses texture 0
  GLuint timeLocation = m_textureProgram.getUniformLocation("time");
  glUniform1f(timeLocation, m_time);

  //grab the camera matrix
  glm::mat4 projectionMatrix = m_camera.getCameraMatrix();
  GLuint pUniform = m_textureProgram.getUniformLocation("P");
  glUniformMatrix4fv(pUniform, 1, GL_FALSE, &(projectionMatrix[0][0]));

  //draw the level
  m_levels[m_currentLevel]->draw();

  //begin drawing agents
  m_agentSpriteBatch.begin();

  m_particleEngine.draw(&m_agentSpriteBatch);

  const glm::vec2 agentDims(AGENT_RADIUS * 2.0f);

  // draw the humans
  for (int i = 0; i < m_humans.size(); i++) {
    if (m_camera.isBoxInView(m_humans[i]->getPosition(), agentDims)) {
      m_humans[i]->draw(m_agentSpriteBatch);
    }
  }

  // draw the zombies
  for (int i = 0; i < m_zombies.size(); i++) {
    if (m_camera.isBoxInView(m_zombies[i]->getPosition(), agentDims)) {
      m_zombies[i]->draw(m_agentSpriteBatch);
    }
  }

  //draw the bullets
  for (int i = 0; i < m_bullets.size(); i++) {
      m_bullets[i].draw(m_agentSpriteBatch);
  }

  m_agentSpriteBatch.end();
  m_agentSpriteBatch.renderBatch();

  drawHud();

  m_textureProgram.unuse();

  m_window.swapBuffer();
}

void MainGame::drawHud() {
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  m_textRenderingProgram.use();

  char buffer[256];

  glActiveTexture(GL_TEXTURE0);
  GLuint fontTextureID = m_spriteFont->getTextureID(); // Add this getter to your SpriteFont class
  glBindTexture(GL_TEXTURE_2D, fontTextureID);
  std::cout << "Binding font texture ID: " << fontTextureID << std::endl;

  GLint textureUniform = m_textRenderingProgram.getUniformLocation("mySampler");
  glUniform1i(textureUniform, 0);

  glm::mat4 projectionMatrix = m_hudCamera.getCameraMatrix();
  GLuint pUniform = m_textRenderingProgram.getUniformLocation("P");
  glUniformMatrix4fv(pUniform, 1, GL_FALSE, &(projectionMatrix[0][0]));

  m_hudSpriteBatch.begin();

  sprintf_s(buffer, "ZOMBIES: %d", m_zombies.size());
  glm::vec2 textPos = glm::vec2(m_screenWidth - 1000, m_screenHeight - 25);
  glm::vec2 textPosWorld = m_hudCamera.convertScreenToWorld(textPos);

  m_spriteFont->draw(m_hudSpriteBatch, buffer, textPosWorld,
    glm::vec2(0.5), 0.0f, JAGEngine::ColorRGBA8(255, 255, 255, 255),
    JAGEngine::Justification::LEFT);

  m_hudSpriteBatch.end();
  m_hudSpriteBatch.renderBatch();

    m_textRenderingProgram.unuse();
}

void MainGame::addBlood(const glm::vec2& position, int numParticles) {
  static std::mt19937 randEngine(time(nullptr));
  static std::uniform_real_distribution<float> randAngle(0.0f, 360.0f);

  glm::vec2 vel(2.0f, 0.0f);
  JAGEngine::ColorRGBA8 col(255, 0, 0, 255);

  for (int i = 0; i < numParticles; i++) {
    m_bloodParticleBatch->addParticle(position, glm::rotate(vel, randAngle(randEngine)), col, 20.0f);
  }
}

void MainGame::initLevel() {

  m_zombies.clear();
  m_bullets.clear();
  std::string levelFileName = "Levels/Level" + std::to_string(m_currentLevel + 1) + ".txt";
  std::cout << "Loading level: " << levelFileName << std::endl;

  try {
    m_levels.push_back(new Level(levelFileName));
    std::cout << "Level object created." << std::endl;

    int width = m_levels[0]->getWidth();
    int height = m_levels[0]->getHeight();
    std::cout << "Level loaded. Width: " << width << ", Height: " << height << std::endl;

    // Initialize player
    if (m_player == nullptr) {
      m_player = new Player();
    }
    m_player->init(PLAYER_SPEED, m_levels[0]->getStartPlayerPos(), &m_inputManager, &m_camera, &m_bullets);
    std::cout << "Player initialized at position: " << m_player->getPosition().x << ", " << m_player->getPosition().y << std::endl;

    // Ensure player is in m_humans vector
    if (m_humans.empty()) {
      m_humans.push_back(m_player);
    }
    else {
      m_humans[0] = m_player;
    }

    // Initialize humans
    std::mt19937 randomEngine(time(nullptr));
    std::uniform_int_distribution<int> randX(2, width - 2);
    std::uniform_int_distribution<int> randY(2, height - 2);

    for (int i = 0; i < m_levels[0]->getNumHumans(); i++) {
      m_humans.push_back(new Human);
      glm::vec2 pos(randX(randomEngine) * TILE_WIDTH, randY(randomEngine) * TILE_WIDTH);
      m_humans.back()->init(HUMAN_SPEED, pos);
    }
    std::cout << "Humans initialized. Total humans: " << m_humans.size() << std::endl;

    // Initialize zombies
    const std::vector<glm::vec2>& zombiePositions = m_levels[0]->getZombieStartPositions();
    for (const auto& pos : zombiePositions) {
      m_zombies.push_back(new Zombie);
      m_zombies.back()->init(ZOMBIE_SPEED, pos);
    }
    std::cout << "Zombies initialized. Total zombies: " << m_zombies.size() << std::endl;

    // Set up player's guns
    m_player->addGun(new Gun("Pistol", 20, 1, 0.15f, 40, 20.0f,
                              m_audioEngine.loadSoundEffect("Sound/shots/pistol1.ogg")));
    m_player->addGun(new Gun("Shotgun", 90, 20, 50.0f, 25, 25.0f,
                              m_audioEngine.loadSoundEffect("Sound/shots/shotgun1.ogg")));
    m_player->addGun(new Gun("MP5", 3, 1, 0.5f, 25, 30.0f,
                              m_audioEngine.loadSoundEffect("Sound/shots/m5.ogg")));
    std::cout << "Player's guns set up." << std::endl;

    std::cout << "Level initialization complete." << std::endl;
  }
  catch (const std::exception& e) {
    std::cerr << "Error in initLevel: " << e.what() << std::endl;
    throw;
  }
}
