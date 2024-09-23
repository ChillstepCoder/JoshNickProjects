#include "Zombie.h"
#include <Bengine/ResourceManager.h>

#include "Human.h"

Zombie::Zombie() {

}
Zombie::~Zombie() {

}

void Zombie::update(const std::vector<std::string>& levelData,
                    std::vector<Human*>& humans,
                    std::vector<Zombie*>& zombies, 
                    float deltaTime) {

    Human* closestHuman = getNearestHuman(humans);

    if (closestHuman != nullptr) {
        glm::vec2 direction = glm::normalize(closestHuman->getPosition() - _position);
        _position += direction * _speed * deltaTime;

        // Calculate rotation in degrees
        _rotation = glm::degrees(atan2(direction.y, direction.x));
    }


    collideWithLevel(levelData);
}

void Zombie::init(float speed, glm::vec2 pos) {
    _speed = speed;
    _position = pos;
    _health = 150;
    _color = Bengine::ColorRGBA8(255, 255, 255, 255);
    _rotation = 0.0f;
}

void Zombie::draw(Bengine::SpriteBatch& _spriteBatch) {

    static int textureID = Bengine::ResourceManager::getTexture("Textures/Zombie/skeleton-attack_0.png").id;

    const glm::vec4 uvRect(0.0f, 0.0f, 1.0f, 1.0f);

    glm::vec4 destRect;
    destRect.x = _position.x;
    destRect.y = _position.y;
    destRect.z = AGENT_WIDTH;
    destRect.w = AGENT_WIDTH;

    _spriteBatch.draw(destRect, uvRect, textureID, 0.0f, _color, _rotation);
}

Human* Zombie::getNearestHuman(std::vector<Human*>& humans) {
    Human* closestHuman = nullptr;
    float smallestDistance = 999999.0f;
    
    for (int i = 0; i < humans.size(); i++) {
        glm::vec2 distVec = humans[i]->getPosition() - _position;
        float distance = glm::length(distVec);
        if (distance < smallestDistance) {
            smallestDistance = distance;
            closestHuman = humans[i];
        }
    }

    return closestHuman;
}