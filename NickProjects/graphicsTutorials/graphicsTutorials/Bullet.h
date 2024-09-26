#pragma once

#include <glm/glm.hpp>
#include <Bengine/SpriteBatch.h>

class Bullet
{
public:
    Bullet(glm::vec2 pos, glm::vec2 dir, float speed, int lifeTime);
    ~Bullet();


    void draw(Bengine::SpriteBatch& spriteBatch);
    //returns true when we are out of lifetime
    bool update();

private:
    int m_lifeTime;
    float m_speed;
    glm::vec2 m_direction;
    glm::vec2 m_position;
};

