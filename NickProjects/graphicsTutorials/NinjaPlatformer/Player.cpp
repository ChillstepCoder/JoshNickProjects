#include "Player.h"
#include <Bengine/ResourceManager.h>
#include <SDL/SDL.h>

Player::Player() {

}

Player::~Player() {

}

void Player::init(b2WorldId* world, const glm::vec2& position, const glm::vec2& dimensions, Bengine::ColorRGBA8 color) {
    Bengine::GLTexture texture = Bengine::ResourceManager::getTexture("Textures/playerRight.png");
    m_collisionBox.init(world, position, dimensions, texture, color, true);
}

void Player::draw(Bengine::SpriteBatch& spriteBatch) {
    m_collisionBox.draw(spriteBatch);
}

void Player::update(Bengine::InputManager& inputManager) {
    if (inputManager.isKeyDown(SDLK_a)) {
        b2Body_ApplyForceToCenter(getID(), b2Vec2(-1000.0f, 0.0f), true);
    } else if (inputManager.isKeyDown(SDLK_d)) {
        b2Body_ApplyForceToCenter(getID(), b2Vec2(1000.0f, 0.0f), true);
    } else if (inputManager.isKeyDown(SDLK_SPACE)) {
        b2Body_ApplyForceToCenter(getID(), b2Vec2(0.0f, 1000.0f), true);
    }
}