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

    void update(Bengine::InputManager& inputManager, const glm::vec2& playerPos, BlockManager* blockManager);

    void movePlayer(float xVelocity, float yVelocity, std::vector<BlockHandle>& blocksInRange, BlockManager* blockManager);

    void setPlayerImage();

    bool checkIntersection(std::vector<BlockHandle>& blocksInRange, BlockManager* blockManager);

    bool intersect(glm::vec2 playerPos1, glm::vec2 playerPos2, glm::vec2 blockPos1, glm::vec2 blockPos2);

    //b2BodyId getID() const { return m_bodyId; }

    float getJumpForce() const { return m_jumpForce; }

    void setJumpForce(float jumpForce) { m_jumpForce = jumpForce; }

    //void setGroundShapeId(b2ShapeId groundShapeId) { m_groundShapeId = groundShapeId; }

private:
    //b2BodyId m_bodyId;
    //b2BodyDef* m_body = nullptr;
    glm::vec2 m_position;
    glm::vec2 m_dimensions;
    glm::vec2 m_velocity;
    float m_horizontalSpeed = 0.3f;
    Bengine::ColorRGBA8 m_color;
    Bengine::GLTexture m_texture;
    std::string m_direction;
    std::string m_image;

    bool m_isGrounded = false;
    bool m_touchingWater = false;
    bool m_facingRight = true;
    //b2ShapeId m_groundShapeId; // Store reference to ground shape
    float m_jumpForce = 1.0f;
    float m_gravity = 0.01f;

    Bengine::Camera2D* m_camera;
    BlockManager* m_blockManager;
};

