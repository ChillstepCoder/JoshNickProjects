#include "Box.h"


Box::Box() {

}
Box::~Box() {

}

void Box::init(b2WorldId* world, const glm::vec2& position, const glm::vec2& dimensions) {
    m_dimensions = dimensions;
    b2BodyDef bodyDef = b2DefaultBodyDef();
    bodyDef.type = b2_dynamicBody;
    bodyDef.position = (b2Vec2)(position.x, position.y);
    b2BodyId bodyId = b2CreateBody(*world, &bodyDef);

    b2Polygon dynamicBox = b2MakeBox(dimensions.x / 2.0f, dimensions.y / 2.0f);

    b2ShapeDef shapeDef = b2DefaultShapeDef();
    shapeDef.density = 1.0f;
    shapeDef.friction = 0.3f;
    b2CreatePolygonShape(bodyId, &shapeDef, &dynamicBox);
}