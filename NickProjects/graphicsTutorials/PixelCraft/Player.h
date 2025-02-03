#pragma once

#include "Block.h"
#include <Bengine/Inputmanager.h>
#include <Bengine/SpriteBatch.h>
#include <Bengine/GLTexture.h>
#include <vector>
#include "BlockMeshManager.h"
#include <Bengine/Camera2D.h>

class Player
{
public:
    Player(); // Default

    Player(Bengine::Camera2D* camera, BlockManager* blockManager);
    ~Player();

    glm::vec2 getPosition();

    const glm::vec2& getDimensions() const { return m_dimensions; }

    void init(b2WorldId* world, const glm::vec2& position, const glm::vec2& dimensions, Bengine::ColorRGBA8 color, Bengine::Camera2D* camera);

    void draw(Bengine::SpriteBatch& spriteBatch);

    void update(Bengine::InputManager& inputManager, const glm::vec2& playerPos, BlockManager* blockManager, bool debugRenderEnabled);

    void movePlayer();

    bool updateCollision(std::vector<BlockHandle>& blocksInRange, BlockManager* blockManager, bool debugRenderEnabled);

    void respawnPlayer();

    float getJumpForce() const { return m_jumpForce; }

    int getScreenIndex() const { return m_screenIndex; }

    void setJumpForce(float jumpForce) { m_jumpForce = jumpForce; }

    void setScreenIndex(int screenIndex) { m_screenIndex = screenIndex; }

private:
    glm::vec2 m_position;
    glm::vec2 m_dimensions;
    glm::vec2 m_velocity = glm::vec2(0.0f);

    float m_maxHorizontalSpeed = 0.4f;
    float m_maxVerticalSpeed = 1.0f;
    float m_horizontalSpeed = 0.0f;
    float m_jumpForce = 0.45f;
    float m_gravity = -0.015f;

    int m_screenIndex = 1;

    Bengine::ColorRGBA8 m_color;
    Bengine::GLTexture m_texturePlayerRight;
    Bengine::GLTexture m_texturePlayerLeft;

    bool m_isGrounded = false;
    bool m_touchingWater = false;
    bool m_isGoingRight = true;

    Bengine::Camera2D* m_camera;
    BlockManager* m_blockManager;
};

