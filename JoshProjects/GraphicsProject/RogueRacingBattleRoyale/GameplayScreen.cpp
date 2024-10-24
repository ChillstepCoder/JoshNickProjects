// GameplayScreen.cpp

#include "GameplayScreen.h"
#include <SDL/SDL.h>
#include <JAGEngine/IMainGame.h>


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

void GameplayScreen::onExit() {
  std::cout << "GameplayScreen::onExit()\n";  // Add debug output

  if (m_physicsSystem) {
    m_physicsSystem->cleanup();
    m_physicsSystem.reset();
  }

  m_trackBodies.clear();
  m_playerCarBody = b2_nullBodyId;
}

void GameplayScreen::onEntry() {
  std::cout << "GameplayScreen::onEntry()\n";  // Add debug output

  try {
    m_physicsSystem = std::make_unique<PhysicsSystem>();
    m_physicsSystem->init(0.0f, -9.81f);

    createTrack();

    // Create player car
    m_playerCarBody = m_physicsSystem->createDynamicBody(0.0f, 4.0f);
    m_physicsSystem->createBoxShape(m_playerCarBody, 2.0f, 1.0f);

    std::cout << "Physics initialization complete\n";  // Add debug output
  }
  catch (const std::exception& e) {
    std::cerr << "Exception in onEntry: " << e.what() << std::endl;
  }
  catch (...) {
    std::cerr << "Unknown exception in onEntry" << std::endl;
  }
}

void GameplayScreen::update() {
  const float timeStep = 1.0f / 60.0f;
  m_physicsSystem->update(timeStep);

  // Update car physics and handle input here
  checkInput();
}

void GameplayScreen::createTrack() {
  // Create ground
  b2BodyId groundBody = m_physicsSystem->createStaticBody(0.0f, -10.0f);
  m_physicsSystem->createBoxShape(groundBody, 100.0f, 20.0f);
  m_trackBodies.push_back(groundBody);

  // Add more track elements as needed
}

void GameplayScreen::checkInput() {
  // Handle car controls here
  if (b2Body_IsValid(m_playerCarBody)) {
    // Example: Apply forces based on input
    // b2Body_ApplyForceToCenter(m_playerCarBody, {forceX, forceY});
  }
}

void GameplayScreen::draw() {
  std::cout << "Draw\n";
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glClearColor(1.0f, 0.0f, 0.0f, 1.0f);

  // Draw track and car here using your rendering system
  // You'll need to get positions and rotations from Box2D bodies
  // and convert them to your rendering coordinates
}
