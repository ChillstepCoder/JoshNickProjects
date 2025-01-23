#include "Player.h"
#include <Bengine/ResourceManager.h>
#include <SDL/SDL.h>
#include "GameplayScreen.h"
#include "DebugDraw.h"
#include <cstdlib> 

#include <iostream>

Player::Player() : m_camera(nullptr) {
    // Default constructor
}

Player::Player(Bengine::Camera2D* camera, BlockManager* blockManager) : m_camera(camera), m_blockManager(blockManager){

}

Player::~Player() {

}

glm::vec2 Player::getPosition() {
    return m_position;
}


void Player::init(b2WorldId* world, const glm::vec2& position, const glm::vec2& dimensions, Bengine::ColorRGBA8 color, Bengine::Camera2D* camera) {
    Bengine::GLTexture texture = Bengine::ResourceManager::getTexture("Textures/playerRight.png");
    m_camera = camera;
    m_position = position;
    m_dimensions = dimensions;
    m_color = color;
    m_texture = texture;

}

void Player::draw(Bengine::SpriteBatch& spriteBatch) {
    glm::vec4 destRect;
    destRect.x = (getPosition().x - ((0.5) * getDimensions().x));
    destRect.y = (getPosition().y - ((0.5) * getDimensions().y));
    destRect.z = getDimensions().x;
    destRect.w = getDimensions().y;
    spriteBatch.draw(destRect, glm::vec4(0.0f, 0.0f, 1.0f, 1.0f), m_texture.id, 0.0f, m_color, 0.0f);
}

void Player::update(Bengine::InputManager& inputManager, const glm::vec2& playerPos, BlockManager* blockManager, bool debugRenderEnabled) {

    if (!(playerPos.x < WORLD_WIDTH_CHUNKS * CHUNK_WIDTH && 0 < playerPos.x && playerPos.y < WORLD_HEIGHT_CHUNKS * CHUNK_WIDTH && 0 < playerPos.y)) {
        respawnPlayer();
        return;
    }

    // Get the blocks in range around the player
    std::vector<BlockHandle> blocksInRange = blockManager->getBlocksInRange(playerPos, 3);

    if (m_velocity.x > 0) {
        m_facingRight = true;
    } else if (m_velocity.x < 0) {
        m_facingRight = false;
    }

    if (!m_facingRight) {
        //m_velocity.x = -1 * m_horizontalSpeed;
        m_direction = "Left";
    }
    else if (m_facingRight) {
        //m_velocity.x = m_horizontalSpeed;
        m_direction = "Right";
    }
    if (m_isGrounded && inputManager.isKeyPressed(SDLK_SPACE)) {
        m_velocity.y = m_jumpForce;
    }

    // Gravity
    m_velocity.y += m_gravity;
    
    // This makes the player string bigger and bigger by adding more text every frame... wtf is this for xD
    //setPlayerImage();


    const float ACCELERATION = 0.02f;
    if (inputManager.isKeyDown(SDLK_a)) {
        m_velocity.x -= ACCELERATION;
    }
    else if (inputManager.isKeyDown(SDLK_d)) {
        m_velocity.x += ACCELERATION;
    }
    else {
        m_velocity.x = (m_velocity.x / 1.2); // slowdown
    }

    movePlayer();
    updateCollision(blocksInRange, blockManager, debugRenderEnabled);

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


void Player::movePlayer() {

    float maxHorizontalSpeed = 0.5f;

    // Clamp is a handy and commonly used function that does what your comment did below :)
    m_velocity = glm::clamp(m_velocity, -maxHorizontalSpeed, maxHorizontalSpeed);
    // TODO: Delete this once you understand
    //if (m_velocity.x > maxHorizontalSpeed) { // set speed to max if it is above
    //    m_velocity.x = maxHorizontalSpeed;
    //} else if (m_velocity.y < -maxHorizontalSpeed) {
    //    m_velocity.y = -maxHorizontalSpeed;
    //}

    m_position += m_velocity;

}


void Player::setPlayerImage() {
    m_image += m_direction;
}

bool Player::updateCollision(std::vector<BlockHandle>& blocksInRange, BlockManager* blockManager, bool debugRenderEnabled) {

    bool hadAnyCollision = false;

    // Reset grounded so we can set it again below
    m_isGrounded = false;

    for (int i = 0; i < blocksInRange.size(); i++) {

        BlockHandle blockHandle = blocksInRange[i];
        if (blockHandle.block->isEmpty()) {
            continue;
        }

        // Get center of the block
        glm::vec2 blockWorldPos = blockHandle.getWorldPosition();

        glm::vec2 blockDimensions(1.0f, 1.0f);


        // DEBUG
        if (debugRenderEnabled) {
            DebugDraw::getInstance().drawBoxAtPoint(b2Vec2(m_position.x, m_position.y), b2Vec2(m_dimensions.x, m_dimensions.y), b2HexColor(b2_colorHotPink), nullptr);

            DebugDraw::getInstance().drawBoxAtPoint(b2Vec2(blockWorldPos.x, blockWorldPos.y), b2Vec2(blockDimensions.x, blockDimensions.y), b2HexColor(b2_colorHotPink), nullptr);

            DebugDraw::getInstance().drawLineBetweenPoints(b2Vec2(m_position.x, m_position.y), b2Vec2(blockWorldPos.x, blockWorldPos.y), b2HexColor(b2_colorHotPink), nullptr);
        }

        float PenetrationDepthY = (((blockDimensions.y / 2) + (m_dimensions.y / 2)) - (abs(blockWorldPos.y - m_position.y)));

        float PenetrationDepthX = (((blockDimensions.x / 2) + (m_dimensions.x / 2)) - (abs(blockWorldPos.x - m_position.x)));

        if (PenetrationDepthY > 0.0f && PenetrationDepthX > 0.0f) {

            if (PenetrationDepthY < PenetrationDepthX) { // collided with top or bottom of a block

                if (blockWorldPos.y < m_position.y) { // collided with the top of the block
                    m_position.y += PenetrationDepthY;
                    m_isGrounded = true;
                    if (m_velocity.y < 0.0f) {
                        m_velocity.y = 0;
                    }

                } else { // collided with the bottom of the block
                    m_position.y -= PenetrationDepthY;

                    if (m_velocity.y > 0.0f) {
                        m_velocity.y = 0;
                    }
                }

            } else { // collided with the side of a block
                if (blockWorldPos.x < m_position.x) { // collided with the right side of a block
                    m_position.x += PenetrationDepthX;
                    if (m_velocity.x < 0.0f) {
                        m_velocity.x = 0;
                    }

                } else { // collided with the left side of a block
                    m_position.x -= PenetrationDepthX;
                    if (m_velocity.x > 0.0f) {
                        m_velocity.x = 0;
                    }
                }
            }
        }
    }
    return hadAnyCollision;

}

void Player::respawnPlayer()
{
    m_position.x = 1024.0f;
    m_position.y = 400.0f;
    m_image = "Player-Idle";
    m_velocity.x = 0;
    m_velocity.y = 0;
    m_isGrounded = false;
    m_gravity = -0.015f;
    m_jumpForce = 1.0f;
    m_horizontalSpeed = 0.0f;
    m_direction = "-L";
}