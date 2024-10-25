// GameplayScreen.cpp
#include "GameplayScreen.h"
#include <SDL/SDL.h>
#include <JAGEngine/IMainGame.h>
#include <GL/glew.h>  

#include <glm/gtc/matrix_transform.hpp>  
#include <glm/gtc/type_ptr.hpp>        

GameplayScreen::GameplayScreen() : m_playerCarBody(b2_nullBodyId) {
}

GameplayScreen::~GameplayScreen() {
}

int GameplayScreen::getNextScreenIndex() const {
  return -1;
}

int GameplayScreen::getPreviousScreenIndex() const {
  return -1;
}

void GameplayScreen::build() {
  // Initialize any resources needed by the screen
  std::cout << "GameplayScreen::build()\n";  // Add debug output
}

void GameplayScreen::destroy() {

}

void GameplayScreen::onEntry() {
  std::cout << "GameplayScreen::onEntry()\n";

  try {
    // Initialize rendering systems
    initShaders();
    m_spriteBatch.init();

    // Load car texture
    m_carTexture = JAGEngine::ResourceManager::getTexture("Textures/car.png").id;
    std::cout << "Loaded car texture with ID: " << m_carTexture << std::endl;

    // Get texture dimensions for debugging
    GLint width, height;
    glBindTexture(GL_TEXTURE_2D, m_carTexture);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);
    std::cout << "Loaded texture dimensions: " << width << "x" << height << std::endl;

    // Get window from parent game
    if (m_game) {
      JAGEngine::IMainGame* game = static_cast<JAGEngine::IMainGame*>(m_game);
      int screenWidth = game->getWindow().getScreenWidth();
      int screenHeight = game->getWindow().getScreenHeight();
      std::cout << "Screen dimensions: " << screenWidth << "x" << screenHeight << std::endl;

      // Use proper scale for projection matrix
      float scaleFactor = 32.0f;
      float aspectRatio = (float)screenWidth / (float)screenHeight;
      m_projectionMatrix = glm::ortho(
        -scaleFactor * aspectRatio, scaleFactor * aspectRatio,
        -scaleFactor, scaleFactor
      );
    }
    else {
      std::cerr << "Game pointer is null!" << std::endl;
      // Use default projection as fallback
      m_projectionMatrix = glm::ortho(-32.0f, 32.0f, -32.0f, 32.0f);
    }

    // Initialize physics
    m_physicsSystem = std::make_unique<PhysicsSystem>();
    m_physicsSystem->init(0.0f, 0.0f);  // No gravity for now

    // Create player car in center
    m_playerCarBody = m_physicsSystem->createDynamicBody(0.0f, 0.0f);
    m_physicsSystem->createBoxShape(m_playerCarBody, 4.0f, 2.0f);

    std::cout << "Initialization complete\n";
  }
  catch (const std::exception& e) {
    std::cerr << "Exception in onEntry: " << e.what() << std::endl;
  }
}

void GameplayScreen::onExit() {
  std::cout << "GameplayScreen::onExit()\n";  // Add debug output

  if (m_physicsSystem) {
    m_physicsSystem->cleanup();
    m_physicsSystem.reset();
  }

  m_trackBodies.clear();
  m_playerCarBody = b2_nullBodyId;
}

void GameplayScreen::draw() {
  checkGLError("start of draw");
  glClearColor(0.1f, 0.1f, 0.1f, 1.0f);  // Slightly lighter background to see if clearing works
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  m_textureProgram.use();
  checkGLError("after shader use");

  // Upload projection matrix
  GLint pUniform = m_textureProgram.getUniformLocation("P");
  glUniformMatrix4fv(pUniform, 1, GL_FALSE, &m_projectionMatrix[0][0]);

  // Begin sprite batch
  m_spriteBatch.begin();

  if (b2Body_IsValid(m_playerCarBody)) {
    b2Vec2 position = b2Body_GetPosition(m_playerCarBody);
    float angle = b2Rot_GetAngle(b2Body_GetRotation(m_playerCarBody));

    // Make the car sprite larger and center it properly
    float carWidth = 4.0f;  // Match physics size
    float carHeight = 2.0f;
    glm::vec4 destRect(
      position.x - carWidth / 2.0f,   // Center horizontally
      position.y - carHeight / 2.0f,   // Center vertically
      carWidth,                      // Width
      carHeight                      // Height
    );

    glm::vec4 uvRect(0.0f, 0.0f, 1.0f, 1.0f);
    JAGEngine::ColorRGBA8 color(255, 255, 255, 255);

    // Debug print
    std::cout << "Drawing car at position: (" << position.x << ", " << position.y
      << ") with dimensions: " << carWidth << "x" << carHeight << std::endl;

    m_spriteBatch.draw(destRect, uvRect, m_carTexture, 0.0f, color, angle);
  }

  m_spriteBatch.end();
  m_spriteBatch.renderBatch();
  checkGLError("after render batch");

  m_textureProgram.unuse();

  // Debug rendering - draw a colored quad in immediate mode to verify rendering is working
  glMatrixMode(GL_PROJECTION);
  glLoadMatrixf(&m_projectionMatrix[0][0]);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glBegin(GL_QUADS);
  glColor3f(1.0f, 0.0f, 0.0f);  // Red color
  glVertex2f(-1.0f, -1.0f);
  glVertex2f(1.0f, -1.0f);
  glVertex2f(1.0f, 1.0f);
  glVertex2f(-1.0f, 1.0f);
  glEnd();
}

void GameplayScreen::update() {
  const float timeStep = 1.0f / 60.0f;
  m_physicsSystem->update(timeStep);
  checkInput();
}

void GameplayScreen::checkInput() {
  if (b2Body_IsValid(m_playerCarBody)) {
    // For now, just keep the car static
    b2Body_SetAwake(m_playerCarBody, true);

    // Later we'll add movement controls here
  }
}

void GameplayScreen::createTrack() {

  // For now, we'll skip track creation
// We'll add walls and boundaries later
// 
  // Create ground
  //b2BodyId groundBody = m_physicsSystem->createStaticBody(0.0f, -10.0f);
  //m_physicsSystem->createBoxShape(groundBody, 100.0f, 20.0f);
  //m_trackBodies.push_back(groundBody);

  // Add more track elements as needed
}

void GameplayScreen::initShaders() {
  // Compile shaders
  m_textureProgram.compileShaders("Shaders/textureShading.vert", "Shaders/textureShading.frag");
  m_textureProgram.addAttribute("vertexPosition");
  m_textureProgram.addAttribute("vertexColor");
  m_textureProgram.addAttribute("vertexUV");
  m_textureProgram.linkShaders();
}

void GameplayScreen::checkGLError(const char* location) {
  GLenum err;
  while ((err = glGetError()) != GL_NO_ERROR) {
    std::cerr << "OpenGL error at " << location << ": " << std::hex << err << std::dec << std::endl;
  }
}
