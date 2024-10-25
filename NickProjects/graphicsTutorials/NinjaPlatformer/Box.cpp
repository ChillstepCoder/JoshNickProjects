#include "Box.h"
#include <iostream>


Box::Box() {

}
Box::~Box() {

}
b2Vec2 Box::getPosition() {
    b2Vec2 position = b2Body_GetPosition(m_ID);
    std::cout << "X: " << position.x << "  Y: " << position.y << std::endl;
    return position;
}

void Box::init(b2WorldId* world, const glm::vec2& position, const glm::vec2& dimensions, Bengine::GLTexture texture, Bengine::ColorRGBA8 color, bool fixedRotation) {
    m_dimensions = dimensions;
    m_color = color;
    m_texture = texture;
    b2BodyDef bodyDef = b2DefaultBodyDef();
    bodyDef.type = b2_dynamicBody;
    bodyDef.position = b2Vec2(position.x, position.y);
    bodyDef.fixedRotation = fixedRotation;
    m_ID = b2CreateBody(*world, &bodyDef);

    b2Polygon dynamicBox = b2MakeBox(dimensions.x / 2.0f, dimensions.y / 2.0f);

    b2ShapeDef shapeDef = b2DefaultShapeDef();
    shapeDef.density = 1.0f;
    shapeDef.friction = 0.3f;
    b2CreatePolygonShape(m_ID, &shapeDef, &dynamicBox);
}

void Box::draw(Bengine::SpriteBatch& spriteBatch) {
    glm::vec4 destRect;
    destRect.x = getPosition().x;
    destRect.y = getPosition().y;
    destRect.z = getDimensions().x;
    destRect.w = getDimensions().y;
    spriteBatch.draw(destRect, glm::vec4(0.0f, 0.0f, 1.0f, 1.0f), m_texture.id, 0.0f, m_color, 0.0f);
}