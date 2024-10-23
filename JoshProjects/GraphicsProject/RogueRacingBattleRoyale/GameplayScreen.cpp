#include "GameplayScreen.h"


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
}

void GameplayScreen::onExit() {

}

void GameplayScreen::update() {
  std::cout << "Update\n";
}

void GameplayScreen::draw() {
  std::cout << "Draw\n";
}
