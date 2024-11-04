#include "Block.h"
#include <iostream>


Block::Block() {

}
Block::~Block() {

}
b2Vec2 Block::getPosition() {
    b2Vec2 position = b2Body_GetPosition(m_ID);
    return position;
}

void Block::init(b2WorldId* world, const glm::vec2& position, const glm::vec2& dimensions, Bengine::GLTexture texture, Bengine::ColorRGBA8 color, bool fixedRotation) {
    m_dimensions = dimensions;
    m_color = color;
    m_texture = texture;
    m_textureID = texture.id;
    m_destRect = glm::vec4(position.x - (0.5 * dimensions.x), position.y - (0.5 * dimensions.y), dimensions.x, dimensions.y);
    b2BodyDef bodyDef = b2DefaultBodyDef();
    bodyDef.position = b2Vec2(position.x, position.y);
    bodyDef.fixedRotation = fixedRotation;
    m_ID = b2CreateBody(*world, &bodyDef);

    b2Polygon dynamicBox = b2MakeBox(dimensions.x / 2.0f, dimensions.y / 2.0f);

    b2ShapeDef shapeDef = b2DefaultShapeDef();
    shapeDef.density = 1.0f;
    shapeDef.friction = 0.0f;
    shapeDef.restitution = 0.0f;
    b2CreatePolygonShape(m_ID, &shapeDef, &dynamicBox);

}

void Block::draw(Bengine::SpriteBatch& spriteBatch) {
    m_destRect.x = (getPosition().x - ((0.5) * getDimensions().x));
    m_destRect.y = (getPosition().y - ((0.5) * getDimensions().y));
    m_destRect.z = getDimensions().x;
    m_destRect.w = getDimensions().y;
    spriteBatch.draw(m_destRect, m_uvRect, m_texture.id, 0.0f, m_color, 0.0f);
}