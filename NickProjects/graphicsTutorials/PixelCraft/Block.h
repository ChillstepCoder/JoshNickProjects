#pragma once
#include <Box2D/box2d.h>
#include <glm/glm.hpp>
#include <Bengine/SpriteBatch.h>
#include <Bengine/GLTexture.h>

class Block
{
public:
    Block();
    ~Block();

    void init(b2WorldId* world, const glm::vec2& position, const glm::vec2& dimensions, Bengine::GLTexture texture, Bengine::ColorRGBA8 color, bool fixedRotation);

    void draw(Bengine::SpriteBatch& spriteBatch);

    b2BodyDef* getBody() const { return m_body; }
    b2Vec2 getPosition();
    b2BodyId getID() const { return m_ID; }
    const glm::vec2& getDimensions() const { return m_dimensions; }

private:
    b2BodyDef* m_body = nullptr;
    b2BodyId m_ID;
    glm::vec2 m_dimensions;
    Bengine::ColorRGBA8 m_color;
    Bengine::GLTexture m_texture;
};