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
    }
    else if (inputManager.isKeyDown(SDLK_d)) {
        b2Body_ApplyForceToCenter(getID(), b2Vec2(1000.0f, 0.0f), true);
    }
    else {
        b2Body_SetLinearVelocity(getID(), b2Vec2(b2Body_GetLinearVelocity(getID()).x * 0.97, b2Body_GetLinearVelocity(getID()).y));
    }

    float MAX_SPEED = 25.0f;
    if (b2Body_GetLinearVelocity(getID()).x < -MAX_SPEED) {
        b2Body_SetLinearVelocity(getID(), b2Vec2(-MAX_SPEED, b2Body_GetLinearVelocity(getID()).y));
    }
    else if (b2Body_GetLinearVelocity(getID()).x > MAX_SPEED) {
        b2Body_SetLinearVelocity(getID(), b2Vec2(MAX_SPEED, b2Body_GetLinearVelocity(getID()).y));
    }

    // Check for ground contact
    m_isGrounded = false;
    b2ContactData contacts[8];
    int contactCount = b2Body_GetContactData(getID(), contacts, 8);

    for (int i = 0; i < contactCount; ++i) {
        // Get the shape IDs from the contact
        b2ShapeId shape1 = contacts[i].shapeIdA;
        b2ShapeId shape2 = contacts[i].shapeIdB;

        // Check if either shape is the ground shape
        if (B2_ID_EQUALS(shape1, m_groundShapeId) || B2_ID_EQUALS(shape2, m_groundShapeId)) {
            m_isGrounded = true;
            break;
        }
    }
    // Jump only when grounded and space is pressed
    if (m_isGrounded && inputManager.isKeyPressed(SDLK_SPACE)) {
        b2Body_ApplyLinearImpulse(getID(), b2Vec2(0.0f, m_jumpForce), b2Vec2(0.0f, 0.0f), true);
    }

}