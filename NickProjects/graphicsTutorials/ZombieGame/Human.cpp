#define GLM_ENABLE_EXPERIMENTAL
#include "Human.h"
#include <ctime>
#include <random>
#include <glm/gtx/rotate_vector.hpp>
#include <Bengine/ResourceManager.h>


Human::Human() :
    _frames(0)
{

}
Human::~Human() {

}

void Human::init(float speed, glm::vec2 pos) {

    static std::mt19937 randomEngine(time(nullptr));
    static std::uniform_real_distribution<float> randDir(-1.0f, 1.0f);
    
    _color.r = 255;
    _color.g = 255;
    _color.b = 255;
    _color.a = 255;

    _speed = speed;
    _position = pos;
    // Get random direction
    _direction = glm::vec2(randDir(randomEngine), randDir(randomEngine));
    // Make sure direction isnt 0
    if (_direction.length() == 0) _direction = glm::vec2(1.0f, 0.0f);

    _direction = glm::normalize(_direction);
}

void Human::update(const std::vector<std::string>& levelData,
                   std::vector<Human*>& humans,
                   std::vector<Zombie*>& zombies) {

    static std::mt19937 randomEngine(time(nullptr));
    static std::uniform_real_distribution<float> randRotate(-1.5f, 1.5f);


    _position += _direction * _speed;

    // Randomly change direction every 40 frames
    if (_frames == 40) {
        _direction = glm::rotate(_direction, randRotate(randomEngine));
        _frames = 0;
    } else {
        _frames++;
    }

    if (collideWithLevel(levelData)) {
        _direction = glm::rotate(_direction, randRotate(randomEngine));
    }
}

void Human::draw(Bengine::SpriteBatch& _spriteBatch) {

    static int textureID = Bengine::ResourceManager::getTexture("Textures/Citizens/Walk1Civ1.png").id;

    const glm::vec4 uvRect(0.0f, 0.0f, 1.0f, 1.0f);

    glm::vec4 destRect;
    destRect.x = _position.x;
    destRect.y = _position.y;
    destRect.z = AGENT_WIDTH;
    destRect.w = AGENT_WIDTH;

    _spriteBatch.draw(destRect, uvRect, textureID, 0.0f, _color);
}