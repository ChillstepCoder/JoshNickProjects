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

    void init(b2WorldId world, const glm::vec2& position, Bengine::GLTexture texture, Bengine::ColorRGBA8 color);

    void draw(Bengine::SpriteBatch& spriteBatch);

    b2Vec2 getPosition();
    b2BodyId getID() const { return m_ID; }
    bool isEmpty() const { return B2_IS_NULL(m_ID); }
    const glm::vec2& getDimensions() const { return m_dimensions; }
    const glm::vec4 getDestRect() const { return m_destRect; }
    const glm::vec4 getUVRect() const { return m_uvRect; }
    const GLuint getTextureID() const { return m_textureID; }
    const Bengine::ColorRGBA8 getColor() const { return m_color; }

private:
    b2BodyId m_ID;

    glm::vec2 m_dimensions;
    glm::vec4 m_destRect;
    glm::vec4 m_uvRect = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
    Bengine::ColorRGBA8 m_color;
    Bengine::GLTexture m_texture;
    GLuint m_textureID;
};