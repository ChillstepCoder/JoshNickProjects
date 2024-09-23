#include "Player.h"
#include <SDL/SDL.h>
#include <Bengine/ResourceManager.h>
#include <iostream>

#include "Gun.h"

Player::Player() :
    _currentGunIndex(-1), _rotation(0.0f) {
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
    _color = Bengine::ColorRGBA8(255, 255, 255, 255);
    _health = 150;
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
                    std::vector<Zombie*>& zombies, 
                    float deltaTime) {

    // Movement controls
    if (_inputManager->isKeyDown(SDLK_w)) {
        _position.y += _speed * deltaTime;
    } else if (_inputManager->isKeyDown(SDLK_s)) {
        _position.y -= _speed * deltaTime;
    }
    if (_inputManager->isKeyDown(SDLK_a)) {
        _position.x -= _speed * deltaTime;
    }
    else if (_inputManager->isKeyDown(SDLK_d)) {
        _position.x += _speed * deltaTime;
    }

    // Weapon switching
    if (_inputManager->isKeyDown(SDLK_1) && _guns.size() >= 0) {
        _currentGunIndex = 0;
    } else if (_inputManager->isKeyDown(SDLK_2) && _guns.size() >= 1) {
        _currentGunIndex = 1;
    } else if (_inputManager->isKeyDown(SDLK_3) && _guns.size() >= 2) {
        _currentGunIndex = 2;
    } else if (_inputManager->isKeyDown(SDLK_4) && _guns.size() >= 3) {
        _currentGunIndex = 3;
    }

    if (_currentGunIndex != -1) {

        glm::vec2 mouseCoords = _inputManager->getMouseCoords();
        mouseCoords = _camera->convertScreenToWorld(mouseCoords);

        glm::vec2 centerPosition = _position + glm::vec2(AGENT_RADIUS); //< Make sure it shoots from the center of the player's body

        glm::vec2 direction = glm::normalize(mouseCoords - centerPosition);

        // Update gun
        _guns[_currentGunIndex]->update(_inputManager->isKeyDown(SDL_BUTTON_LEFT),
                centerPosition,
                direction,
                *_bullets,
                deltaTime);
    }

    collideWithLevel(levelData);

    // Calculate the rotation angle of the character
    glm::vec2 mouseCoords = _inputManager->getMouseCoords();
    mouseCoords = _camera->convertScreenToWorld(mouseCoords);

    glm::vec2 direction = glm::normalize(mouseCoords - _position);
    _rotation = atan2f(direction.y, direction.x); // Angle in radians
}

void Player::draw(Bengine::SpriteBatch& _spriteBatch) {

    static int textureID = Bengine::ResourceManager::getTexture("Textures/Top_Down_Survivor/rifle/idle/survivor-idle_rifle_0.png").id;

    const glm::vec4 uvRect(0.0f, 0.0f, 1.0f, 1.0f);

    glm::vec4 destRect;
    destRect.x = _position.x;
    destRect.y = _position.y;
    destRect.z = AGENT_WIDTH;
    destRect.w = AGENT_WIDTH;

    _spriteBatch.draw(destRect, uvRect, textureID, 0.0f, _color, glm::degrees(_rotation));
}