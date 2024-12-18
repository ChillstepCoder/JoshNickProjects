#include "Player.h"
#include <Bengine/ResourceManager.h>
#include <SDL/SDL.h>
#include "GameplayScreen.h"

Player::Player() : m_camera(nullptr) {
    // Default constructor
}

Player::Player(Bengine::Camera2D* camera, BlockManager* blockManager) : m_camera(camera), m_blockManager(blockManager) {

}

Player::~Player() {

}

b2Vec2 Player::getPosition() {
    b2Vec2 position = b2Body_GetPosition(m_bodyId);
    return position;
}


void Player::init(b2WorldId* world, const glm::vec2& position, const glm::vec2& dimensions, Bengine::ColorRGBA8 color, Bengine::Camera2D* camera) {
    Bengine::GLTexture texture = Bengine::ResourceManager::getTexture("Textures/playerRight.png");
    m_camera = camera;
    m_position = position;
    m_dimensions = dimensions;
    m_color = color;
    m_texture = texture;
    b2BodyDef bodyDef = b2DefaultBodyDef();
    bodyDef.type = b2_dynamicBody;
    bodyDef.position = b2Vec2(position.x, position.y);
    bodyDef.fixedRotation = true;
    m_bodyId = b2CreateBody(*world, &bodyDef);

    float radius = dimensions.x / 2.0f; // Use half the width as the radius
    float height = dimensions.y - radius * 2; // The height of the rectangle part

    b2Capsule capsule;
    capsule.radius = radius;
    capsule.center1 = b2Vec2(0.0f, dimensions.y / 3.5f);
    capsule.center2 = b2Vec2(0.0f, -dimensions.y / 3.5f);
    b2ShapeDef capsuleShapeDef = b2DefaultShapeDef();
    capsuleShapeDef.density = 1.0f;
    capsuleShapeDef.friction = 0.1f;
    capsuleShapeDef.restitution = 0.0f;
    b2CreateCapsuleShape(m_bodyId, &capsuleShapeDef, &capsule);

}

void Player::draw(Bengine::SpriteBatch& spriteBatch) {
    glm::vec4 destRect;
    destRect.x = (getPosition().x - ((0.5) * getDimensions().x));
    destRect.y = (getPosition().y - ((0.5) * getDimensions().y));
    destRect.z = getDimensions().x;
    destRect.w = getDimensions().y;
    spriteBatch.draw(destRect, glm::vec4(0.0f, 0.0f, 1.0f, 1.0f), m_texture.id, 0.0f, m_color, 0.0f);
}

void Player::update(Bengine::InputManager& inputManager, const glm::vec2& playerPos, BlockManager* blockManager) {
    // Get the blocks in range around the player
    std::vector<Block> blocksInRange = blockManager->getBlocksInRange(playerPos, 2);  // Use a range of 2 chunks


    // Check for ground and water contact
    //m_touchingWater = false;
    m_isGrounded = false;
    // Check contact with each block
    for (const auto& block : blocksInRange) {
        b2ContactData contacts[8];
        int contactCount = b2Body_GetContactData(getID(), contacts, 8);

        for (int i = 0; i < contactCount; ++i) {
            b2ShapeId shape1 = contacts[i].shapeIdA;
            b2ShapeId shape2 = contacts[i].shapeIdB;

            // Check if the block's shape ID matches the player's shape ID
            if (B2_ID_EQUALS(shape1, block.getBodyID()) || B2_ID_EQUALS(shape2, block.getBodyID())) {
                m_isGrounded = true; // Player is grounded on this block
                break; // No need to check further
            }
        }

        if (m_isGrounded) break; // Exit if already grounded
    }


    // Handle movement input
    float force = m_touchingWater ? 20.0f : 50.0f; // Define a base force for movement


    if (inputManager.isKeyDown(SDLK_a)) {
        b2Body_ApplyForceToCenter(getID(), b2Vec2(-force, 0.0f), true);
    }
    else if (inputManager.isKeyDown(SDLK_d)) {
        b2Body_ApplyForceToCenter(getID(), b2Vec2(force, 0.0f), true);
    }
    else {
        // Apply gradual slowdown
        b2Vec2 currentVelocity = b2Body_GetLinearVelocity(getID());
        float slowdownFactor = m_isGrounded ? 0.84f : 0.88f; // Slower when grounded
        b2Body_SetLinearVelocity(getID(), b2Vec2(currentVelocity.x * slowdownFactor, currentVelocity.y));
    }

    // Enforce maximum speed
    float MAX_SPEED = m_touchingWater ? 10.0f : 25.0f;
    b2Vec2 currentVelocity = b2Body_GetLinearVelocity(getID());
    if (currentVelocity.x < -MAX_SPEED) {
        b2Body_SetLinearVelocity(getID(), b2Vec2(-MAX_SPEED, currentVelocity.y));
    } else if (currentVelocity.x > MAX_SPEED) {
        b2Body_SetLinearVelocity(getID(), b2Vec2(MAX_SPEED, currentVelocity.y));
    }
    if (currentVelocity.y < -MAX_SPEED * 2) {
        b2Body_SetLinearVelocity(getID(), b2Vec2(currentVelocity.x, -MAX_SPEED * 2));
    }
    else if (currentVelocity.y > MAX_SPEED * 2) {
        b2Body_SetLinearVelocity(getID(), b2Vec2(currentVelocity.x, MAX_SPEED * 2));
    }

    float realJumpForce = m_touchingWater ? m_jumpForce * 0.05f  : m_jumpForce;
    // Jump only when grounded or in water and space is pressed
    if ((m_isGrounded && inputManager.isKeyPressed(SDLK_SPACE))) {
        b2Body_ApplyLinearImpulse(getID(), b2Vec2(0.0f, realJumpForce), b2Body_GetPosition(getID()), false);
    }

    BlockHandle blockHandle = blockManager->getBlockAtPosition(playerPos);
    if (blockHandle.block->getBlockID() == BlockID::WATER && inputManager.isKeyDown(SDLK_SPACE)) {
        m_touchingWater = true;
        b2Body_SetGravityScale(getID(), 0.35f);
        b2Body_ApplyLinearImpulse(getID(), b2Vec2(0.0f, realJumpForce), b2Body_GetPosition(getID()), false);
    } else if (blockHandle.block->getBlockID() == BlockID::WATER) {
        m_touchingWater = true;
        b2Body_SetGravityScale(getID(), 0.35f);
    } else {
        m_touchingWater = false;
        b2Body_SetGravityScale(getID(), 1.00f);
    }

    // Handle block breaking
    if (inputManager.isKeyDown(SDL_BUTTON_LEFT)) {
        // Get the player's mouse position
        glm::vec2 mouseCoords = inputManager.getMouseCoords();

        // Convert mouse position to world coordinates
        glm::vec2 mouseWorldPos = m_camera->convertScreenToWorld(mouseCoords);

        // Check if the mouse position is over a block and break it
        m_blockManager->breakBlockAtPosition(mouseWorldPos, playerPos);
    }

    if (inputManager.isKeyDown(SDL_BUTTON_RIGHT)) {
        // Get the player's mouse position
        glm::vec2 mouseCoords = inputManager.getMouseCoords();

        // Convert mouse position to world coordinates
        glm::vec2 mouseWorldPos = m_camera->convertScreenToWorld(mouseCoords);

        // Check if the mouse position is over a block and break it
        m_blockManager->placeBlockAtPosition(mouseWorldPos, playerPos);
    }
}