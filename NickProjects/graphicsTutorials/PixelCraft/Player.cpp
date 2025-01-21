#include "Player.h"
#include <Bengine/ResourceManager.h>
#include <SDL/SDL.h>
#include "GameplayScreen.h"
#include "DebugDraw.h"

Player::Player() : m_camera(nullptr) {
    // Default constructor
}

Player::Player(Bengine::Camera2D* camera, BlockManager* blockManager) : m_camera(camera), m_blockManager(blockManager){

}

Player::~Player() {

}

glm::vec2 Player::getPosition() {

    glm::vec2 position = m_position;
    return position;
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
    std::vector<BlockHandle> blocksInRange = blockManager->getBlocksInRange(playerPos, 2);  // Use a range of 2 chunks



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

    if (m_isGrounded) {
        if (m_velocity.y < 0) {
            m_isGrounded = false;
        }
        if (inputManager.isKeyPressed(SDLK_SPACE)) {
            m_velocity.y = -1 * m_jumpForce;
        }
    }

    movePlayer(m_velocity.x, 0, blocksInRange, blockManager, debugRenderEnabled);


    if (m_isGrounded == false) {
        m_velocity.y += m_gravity;
    }
    movePlayer(0, m_velocity.y, blocksInRange, blockManager, debugRenderEnabled);
    setPlayerImage();


    if (inputManager.isKeyDown(SDLK_a)) {
        m_velocity.x -= 0.02f;
    }
    else if (inputManager.isKeyDown(SDLK_d)) {
        m_velocity.x += 0.02f;
    }
    else {
        m_velocity.x = (m_velocity.x / 1.2); // slowdown
    }

    if ((m_isGrounded && inputManager.isKeyPressed(SDLK_SPACE))) {
        m_velocity.y = 0.4f;
        m_isGrounded = false;
        movePlayer(0, m_velocity.y, blocksInRange, blockManager, debugRenderEnabled);
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


void Player::movePlayer(float xVelocity, float yVelocity, std::vector<BlockHandle>& blocksInRange, BlockManager* blockManager, bool debugRenderEnabled) {

    float maxHorizontalSpeed = 0.5f;

    if (xVelocity > maxHorizontalSpeed) { // set speed to max if it is above
        xVelocity = maxHorizontalSpeed;
    } else if (xVelocity < -maxHorizontalSpeed) {
        xVelocity = -maxHorizontalSpeed;
    }


    m_position.y += yVelocity;
    m_position.x += xVelocity;

    bool intersect = checkIntersection(blocksInRange, blockManager, debugRenderEnabled);

    if (!intersect) {
        return;
    }
    if (xVelocity == 0) {
        yVelocity = 0;
    }
}


void Player::setPlayerImage() {
    m_image += m_direction;
}

bool Player::checkIntersection(std::vector<BlockHandle>& blocksInRange, BlockManager* blockManager, bool debugRenderEnabled) {

    for (int i = 0; i < blocksInRange.size(); i++) {

        BlockHandle blockHandle = blocksInRange[i];

        glm::vec2 blockWorldPos = blockHandle.getWorldPosition();

        glm::vec2 blockDimensions(1.0f, 1.0f);

        //glm::vec2 correctedPlayerPosition = glm::vec2(m_position.x - (m_dimensions.x * 0.5), (m_position.y - (m_dimensions.y * 0.5))); // incorrect for some reason
        glm::vec2 correctedPlayerPosition = glm::vec2(m_position.x - 0.25, (m_position.y - 0.95));

        if (intersect(correctedPlayerPosition, correctedPlayerPosition + m_dimensions, blockWorldPos, blockWorldPos + blockDimensions)) {
            // DEBUG
            if (debugRenderEnabled) {
                DebugDraw::getInstance().drawBoxAtPoint(b2Vec2(correctedPlayerPosition.x, correctedPlayerPosition.y), b2Vec2(m_dimensions.x, m_dimensions.y), b2HexColor(b2_colorHotPink), nullptr);

                DebugDraw::getInstance().drawBoxAtPoint(b2Vec2(blockWorldPos.x, blockWorldPos.y), b2Vec2(blockDimensions.x, blockDimensions.y), b2HexColor(b2_colorHotPink), nullptr);

                DebugDraw::getInstance().drawLineBetweenPoints(b2Vec2(correctedPlayerPosition.x, correctedPlayerPosition.y), b2Vec2(blockWorldPos.x, blockWorldPos.y), b2HexColor(b2_colorHotPink), nullptr);
            }

            // Handle bottom collision (falling through the block)
            if ((blockWorldPos.y + blockDimensions.y) > correctedPlayerPosition.y) {
                m_isGrounded = true;
                m_velocity.y = 0;
                m_position.y = (blockWorldPos.y + blockDimensions.y + (0.9f));
            } else if ((blockWorldPos.x + blockDimensions.x > correctedPlayerPosition.x 
                && blockWorldPos.y > correctedPlayerPosition.y 
                && blockWorldPos.y < (correctedPlayerPosition.y + m_dimensions.y)) 
                || (blockWorldPos.x < correctedPlayerPosition.x + m_dimensions.x 
                && blockWorldPos.y > correctedPlayerPosition.y 
                && blockWorldPos.y < (correctedPlayerPosition.y + m_dimensions.y))) {
                if (m_velocity.x > 0) { // Moving right
                    m_position.x = blockWorldPos.x - m_dimensions.x; // Move player out of block
                    m_velocity.x = 0; // Stop horizontal movement
                }
                else if (m_velocity.x < 0) { // Moving left
                    m_position.x = blockWorldPos.x + blockDimensions.x; // Move player out of block
                    m_velocity.x = 0; // Stop horizontal movement
                }
            }
            return true;
        }
    }
    m_isGrounded = false;
    return false;

}

bool Player::intersect(glm::vec2 playerPos1, glm::vec2 playerPos2, glm::vec2 blockPos1, glm::vec2 blockPos2)
{
    return playerPos1.x < blockPos2.x && playerPos2.x > blockPos1.x && playerPos1.y < blockPos2.y && playerPos2.y > blockPos1.y;
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