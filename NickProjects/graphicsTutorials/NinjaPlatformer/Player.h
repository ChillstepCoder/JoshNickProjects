#pragma once

#include "Box.h"
#include <Bengine/Inputmanager.h>
#include <Bengine/SpriteBatch.h>
#include <Bengine/GLTexture.h>


class Player
{
public:
    Player();
    ~Player();

    void init(b2WorldId* world, const glm::vec2& position, const glm::vec2& dimensions, Bengine::ColorRGBA8 color);

    void draw(Bengine::SpriteBatch& spriteBatch);

    void update(Bengine::InputManager& inputManager);

    const Box& getBox() const { return m_collisionBox; }

    b2BodyId getID() const { return m_collisionBox.getID(); }

    void setGroundShapeId(b2ShapeId groundShapeId) { m_groundShapeId = groundShapeId; }

private:
    Box m_collisionBox;
    bool m_isGrounded = false;
    b2ShapeId m_groundShapeId; // Store reference to ground shape
    float m_jumpForce = 1000.0f;
};

