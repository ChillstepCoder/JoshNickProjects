#include "GameplayScreen.h"
#include <SDL/SDL.h>
#include <JAGEngine/IMainGame.h>

GameplayScreen::GameplayScreen() {
  // empty
}
GameplayScreen::~GameplayScreen() {
  // empty
}

int GameplayScreen::getNextScreenIndex() const {
  return -1;
}

int GameplayScreen::getPreviousScreenIndex() const {
  return -1;
}

void GameplayScreen::build() {

}

void GameplayScreen::destroy() {

}

void GameplayScreen::onEntry() {
  std::cout << "On Entry\n";

  //b2Vec2 gravity(0.0f, -9.81f);
  //m_world = std::make_unique<b2World>(gravity);

  //// make the ground
  //b2BodyDef groundBodyDef;
  //groundBodyDef.position.Set(0.0f, -10.0f);
  //b2Body* groundBody = m_world->CreateBody(&groundBodyDef);

  //// make the ground fixture.
  //b2PolygonShape groundBox;
  //groundBox.SetAsBox(50.0f, 10.0f);
  //groundBody->CreateFixture(&groundBody, 0.0f);
}

void GameplayScreen::onExit() {

}

void GameplayScreen::update() {
  std::cout << "Update\n";
}

void GameplayScreen::draw() {
  std::cout << "Draw\n";
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
}

void GameplayScreen::checkInput() {

}
