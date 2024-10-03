#include "Bullet.h"
#include <Bengine/ResourceManager.h>


Bullet::Bullet(glm::vec2 pos, glm::vec2 dir, float speed, int lifeTime) {
    m_lifeTime = lifeTime;
    m_position = pos;
    m_direction = dir;
    m_speed = speed;
}
Bullet::~Bullet() {

}

void Bullet::draw(Bengine::SpriteBatch& spriteBatch) {
    glm::vec4 uv(0.0f, 0.0f, 1.0f, 1.0f);
    static Bengine::GLTexture texture = Bengine::ResourceManager::getTexture("Textures/jimmyJump_pack/PNG/Bullet.png");
    Bengine::ColorRGBA8 color;
    color.r = 255;
    color.g = 255;
    color.b = 255;
    color.a = 255;

    glm::vec4 posAndSize = glm::vec4(m_position.x, m_position.y, 10, 10);

    spriteBatch.draw(posAndSize, uv, texture.id, 0.0f, color, 0.0f);
}

bool Bullet::update() {
    m_position += m_direction * m_speed;
    m_lifeTime--;
    if (m_lifeTime == 0) {
        return true;
    }
    return false;
}
