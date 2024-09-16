#include "Bullet.h"
#include <Bengine/Resourcemanager.h>


Bullet::Bullet(glm::vec2 position, glm::vec2 direction, float damage, float speed) :
    _position(position),
    _direction(direction),
    _damage(damage),
    _speed(speed) {
    // Empty
}

Bullet::~Bullet() {
    // Empty
}

void Bullet::update(std::vector<Human*>& humans,
    std::vector<Zombie*>& zombies) {

    _position += _direction * _speed;
}

void Bullet::draw(Bengine::SpriteBatch& spriteBatch) {
    glm::vec4 destRect(_position.x + BULLET_RADIUS,
                       _position.y + BULLET_RADIUS,
                       BULLET_RADIUS * 2,
                       BULLET_RADIUS * 2);
    const glm::vec4 uvRect(0.0f, 0.0f, 1.0f, 1.0f);

    Bengine::Color color;
    color.r = 255;
    color.g = 255;
    color.b = 255;
    color.a = 255;

    spriteBatch.draw(destRect, uvRect, Bengine::ResourceManager::getTexture("Textures/Bullet/bullet.png").id, 0.0f, color);
}