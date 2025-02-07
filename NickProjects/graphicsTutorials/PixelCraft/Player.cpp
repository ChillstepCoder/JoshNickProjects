#include "Player.h"
#include <Bengine/ResourceManager.h>
#include <SDL/SDL.h>
#include "GameplayScreen.h"
#include "DebugDraw.h"
#include <cstdlib> 

#include <iostream>

const float WATER_GRAVITY = -0.005f;
const float WATER_JUMP_FORCE = 0.30f;
const float WATER_MAX_HORIZONTAL_SPEED = 0.25f;
const float WATER_MAX_VERTICAL_SPEED = 0.3f;


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
    Bengine::GLTexture playerRight = Bengine::ResourceManager::getTexture("Textures/playerRight.png", Bengine::TextureFilterMode::Nearest);
    Bengine::GLTexture playerLeft = Bengine::ResourceManager::getTexture("Textures/playerLeft.png", Bengine::TextureFilterMode::Nearest);
    m_camera = camera;
    m_position = position;
    m_dimensions = dimensions;
    m_color = color;
    m_texturePlayerRight = playerRight;
    m_texturePlayerLeft = playerLeft;

}

void Player::draw(Bengine::SpriteBatch& spriteBatch) {
    glm::vec4 destRect;

    GLuint textureId;
    if (m_isGoingRight) {
        textureId = m_texturePlayerRight.id;
    } else {
        textureId = m_texturePlayerLeft.id;
    }

    destRect.x = (getPosition().x - ((0.5) * getDimensions().x));
    destRect.y = (getPosition().y - ((0.5) * getDimensions().y));
    destRect.z = getDimensions().x;
    destRect.w = getDimensions().y;
    spriteBatch.draw(destRect, glm::vec4(0.0f, 0.0f, 1.0f, 1.0f), textureId, 0.0f, m_color, 0.0f);
}

void Player::update(Bengine::InputManager& inputManager, const glm::vec2& playerPos, BlockManager* blockManager, bool debugRenderEnabled) {

    if (!(playerPos.x < WORLD_WIDTH_CHUNKS * CHUNK_WIDTH && 0 < playerPos.x && playerPos.y < WORLD_HEIGHT_CHUNKS * CHUNK_WIDTH && 0 < playerPos.y)) {
        respawnPlayer();
        return;
    }

    // Get the blocks in range around the player
    std::vector<BlockHandle> blocksInRange = blockManager->getBlocksInRange(playerPos, 2);

    if (m_velocity.x > 0) {
        m_isGoingRight = true;
    } else if (m_velocity.x < 0) {
        m_isGoingRight = false;
    }

    if (m_isGrounded && inputManager.isKeyPressed(SDLK_SPACE)) {
        m_velocity.y = m_jumpForce;
    }

    // Gravity
    m_velocity.y += m_gravity;

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

    if (inputManager.isKeyPressed(SDLK_ESCAPE)) {
        ImGui::Begin("Options");

        ImGui::NewFrame();

        static int volume = 0;
        if (ImGui::Button("Volume"))
            volume++;

        if (volume & 1)
        {
            ImGui::SameLine();
            ImGui::Text("volume things happening!!!!");
        }
        static int graphics = 0;
        if (ImGui::Button("Graphics"))
            graphics++;
        if (graphics & 1)
        {
            ImGui::SameLine();
            ImGui::Text("graphics have been changed. JK LOL!");
        }
        int menu = 0;
        if (ImGui::Button("Return to Menu"))
            menu++;
        if (menu & 1)
        {
            setScreenIndex(0);
        }

        ImGui::End();
    }

}


void Player::movePlayer() {

    m_velocity.x = glm::clamp(m_velocity.x, -m_maxHorizontalSpeed, m_maxHorizontalSpeed);

    m_velocity.y = glm::clamp(m_velocity.y, -m_maxVerticalSpeed, m_maxVerticalSpeed);

    m_position += m_velocity;

}

bool Player::updateCollision(std::vector<BlockHandle>& blocksInRange, BlockManager* blockManager, bool debugRenderEnabled) {

    bool hadAnyCollision = false;

    // Reset variables so we can set them again below
    m_gravity = -0.015f;
    m_jumpForce = 0.45f;
    m_maxHorizontalSpeed = 0.4f;
    m_maxVerticalSpeed = 1.0f;
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

            if (blockHandle.block->getBlockID() == BlockID::WATER) { // touching water
                m_gravity = WATER_GRAVITY;
                m_jumpForce = WATER_JUMP_FORCE;
                m_maxHorizontalSpeed = WATER_MAX_HORIZONTAL_SPEED;
                m_maxVerticalSpeed = WATER_MAX_VERTICAL_SPEED;
                m_isGrounded = true;
                continue;
            }


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
    m_velocity.x = 0;
    m_velocity.y = 0;
    m_isGrounded = false;
    m_gravity = -0.015f;
    m_jumpForce = 1.0f;
    m_horizontalSpeed = 0.0f;
}