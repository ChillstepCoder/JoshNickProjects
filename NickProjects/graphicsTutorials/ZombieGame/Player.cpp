#include "Player.h"
#include <SDL/SDL.h>
#include <Bengine/ResourceManager.h>

#include "Gun.h"

Player::Player() :
    _currentGunIndex(-1) {
    // Empty
}
Player::~Player() {
    // Empty
}

void Player::init(float speed, glm::vec2 pos, Bengine::InputManager* inputManager, Bengine::Camera2D* camera, std::vector<Bullet>* bullets) {
    _speed = speed;
    _position = pos;
    _inputManager = inputManager;
    _bullets = bullets;
    _camera = camera;
    _color.r = 255;
    _color.g = 255;
    _color.b = 255;
    _color.a = 255;
}

void Player::addGun(Gun* gun) {
    // Add the gun to player inventory
    _guns.push_back(gun);

    // If no gun equipped, equip gun.
    if (_currentGunIndex == -1) {
        _currentGunIndex = 0;
    }
}

void Player::update(const std::vector<std::string>& levelData,
                    std::vector<Human*>& humans,
                    std::vector<Zombie*>& zombies) {

    if (_inputManager->isKeyPressed(SDLK_w)) {
        _position.y += _speed;
    } else if (_inputManager->isKeyPressed(SDLK_s)) {
        _position.y -= _speed;
    }
    if (_inputManager->isKeyPressed(SDLK_a)) {
        _position.x -= _speed;
    }
    else if (_inputManager->isKeyPressed(SDLK_d)) {
        _position.x += _speed;
    }

    if (_inputManager->isKeyPressed(SDLK_1) && _guns.size() >= 0) {
        _currentGunIndex = 0;
    } else if (_inputManager->isKeyPressed(SDLK_2) && _guns.size() >= 1) {
        _currentGunIndex = 1;
    } else if (_inputManager->isKeyPressed(SDLK_3) && _guns.size() >= 2) {
        _currentGunIndex = 2;
    }

    if (_currentGunIndex != -1) {

        glm::vec2 mouseCoords = _inputManager->getMouseCoords();
        mouseCoords = _camera->convertScreenToWorld(mouseCoords);

        glm::vec2 centerPosition = _position + glm::vec2(AGENT_RADIUS); //< Make sure it shoots from the center of the player's body

        glm::vec2 direction = glm::normalize(mouseCoords - centerPosition);

        _guns[_currentGunIndex]->update(_inputManager->isKeyPressed(SDL_BUTTON_LEFT),
                centerPosition,
                direction,
                *_bullets);
    }

    collideWithLevel(levelData);
}

void Player::draw(Bengine::SpriteBatch& _spriteBatch) {

    static int textureID = Bengine::ResourceManager::getTexture("Textures/Top_Down_Survivor/rifle/idle/survivor-idle_rifle_0.png").id;

    const glm::vec4 uvRect(0.0f, 0.0f, 1.0f, 1.0f);

    glm::vec4 destRect;
    destRect.x = _position.x;
    destRect.y = _position.y;
    destRect.z = AGENT_WIDTH;
    destRect.w = AGENT_WIDTH;

    _spriteBatch.draw(destRect, uvRect, textureID, 0.0f, _color);
}