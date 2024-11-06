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

    b2Vec2 getPosition();

    const glm::vec2& getDimensions() const { return m_dimensions; }

    void init(b2WorldId* world, const glm::vec2& position, const glm::vec2& dimensions, Bengine::ColorRGBA8 color, Bengine::Camera2D* camera);

    void draw(Bengine::SpriteBatch& spriteBatch);

    void update(Bengine::InputManager& inputManager, const std::vector<Block>& blocks);

    b2BodyId getID() const { return m_bodyId; }

    float getJumpForce() const { return m_jumpForce; }

    void setJumpForce(float jumpForce) { m_jumpForce = jumpForce; }

    void setGroundShapeId(b2ShapeId groundShapeId) { m_groundShapeId = groundShapeId; }

private:
    b2BodyId m_bodyId;
    b2BodyDef* m_body = nullptr;
    glm::vec2 m_dimensions;
    Bengine::ColorRGBA8 m_color;
    Bengine::GLTexture m_texture;

    bool m_isGrounded = false;
    b2ShapeId m_groundShapeId; // Store reference to ground shape
    float m_jumpForce = 2200.0f;

    Bengine::Camera2D* m_camera;
    BlockManager* m_blockManager;
};

