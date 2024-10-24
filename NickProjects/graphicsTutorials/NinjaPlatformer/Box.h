#pragma once
#include <Box2D/box2d.h>
#include <glm/glm.hpp>

class Box
{
public:
    Box();
    ~Box();

    void init(b2WorldId* world, const glm::vec2& position, const glm::vec2& dimensions);

    b2BodyDef* getBody() const { return m_body; }
    b2Vec2 getPosition();
    b2BodyId getID() const { return m_ID;  }
    const glm::vec2& getDimensions() const { return m_dimensions; }

private:
    b2BodyDef* m_body = nullptr;
    b2BodyId m_ID;

    glm::vec2 m_dimensions;
};