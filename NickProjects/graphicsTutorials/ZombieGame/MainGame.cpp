#include "MainGame.h"

#include <Bengine/Bengine.h>
#include <Bengine/Timing.h>
#include <random>
#include <ctime>
#include "Bengine/Errors.h"

#include <SDL/SDL.h>
#include <iostream>

#include "Gun.h"
#include "Zombie.h"

const float HUMAN_SPEED = 1.0f;
const float ZOMBIE_SPEED = 1.3f;
const float PLAYER_SPEED = 5.0f;

MainGame::MainGame()  : 
    _screenWidth(1024), 
    _screenHeight(768),
    _gameState(GameState::PLAY),
    _fps(0.0f),
    _player(nullptr) {
    // Empty
}

MainGame::~MainGame() {
    for (int i = 0; i < _levels.size(); i++) {
        delete _levels[i];
    }
}

void MainGame::run() {
    initSystems();

    initLevel();

    gameLoop();

}

void MainGame::initSystems() {
    Bengine::init();

    _window.create("ZombieGame", _screenWidth, _screenHeight, 0);
    glClearColor(0.7f, 0.7f, 0.7f, 1.0f);

    initShaders();

    _agentSpriteBatch.init();

    _camera.init(_screenWidth, _screenHeight);

}

void MainGame::initLevel() {
    // Level 1
    _levels.push_back(new Level("Levels/level1.txt"));
    _currentLevel = 0;

    _player = new Player();
    _player->init(PLAYER_SPEED, _levels[_currentLevel]->getStartPlayerPos(), &_inputManager, &_camera, &_bullets);

    _humans.push_back(_player);

    std::mt19937 randomEngine;
    randomEngine.seed(time(nullptr));


    std::uniform_int_distribution<int> randX(3, _levels[_currentLevel]->getWidth() - 3);
    std::uniform_int_distribution<int> randY(3, _levels[_currentLevel]->getHeight() - 3);

    // Add all the random humans
    for (int i = 0; i < _levels[_currentLevel]->getNumHumans(); i++) {
        _humans.push_back(new Human);
        glm::vec2 pos(randX(randomEngine) * TILE_WIDTH, randY(randomEngine) * TILE_WIDTH);
        _humans.back()->init(HUMAN_SPEED, pos);
    }

    // Add the zombies
    const std::vector<glm::vec2>& zombiePositions = _levels[_currentLevel]->getZombieStartPositions();
    for (int i = 0; i < zombiePositions.size(); i++) {
        _zombies.push_back(new Zombie);
        _zombies.back()->init(ZOMBIE_SPEED, zombiePositions[i]);
    }

    // Set up the player's guns
    const float BULLET_SPEED = 20.0f;
    _player->addGun(new Gun("Magnum", 15, 1, 0.1f, 30, BULLET_SPEED));
    _player->addGun(new Gun("Shotgun", 35, 20, 0.4f, 4, BULLET_SPEED));
    _player->addGun(new Gun("AK-47", 4, 1, 0.15f, 20, BULLET_SPEED));
}

void MainGame::initShaders() {
    // Compile our color shader
    _textureProgram.compileShaders("Shaders/textureShadingVert.txt", "Shaders/textureShadingFrag.txt");
    _textureProgram.addAttribute("vertexPosition");
    _textureProgram.addAttribute("vertexColor");
    _textureProgram.addAttribute("vertexUV");
    _textureProgram.linkShaders();
}

void MainGame::gameLoop() {
    
    Bengine::FpsLimiter fpsLimiter;
    fpsLimiter.setMaxFPS(60.0f);


    while (_gameState == GameState::PLAY) {
        fpsLimiter.begin();

        processInput();

        updateAgents();

        updateBullets();

        _camera.setPosition(_player->getPosition());

        _camera.update();

        drawGame();

        _fps = fpsLimiter.end();
    }
}

void MainGame::updateAgents() {
    // Update all humans
    for (int i = 0; i < _humans.size(); i++) {
        _humans[i]->update(_levels[_currentLevel]->getLevelData(),
                           _humans,
                           _zombies);
    }

    // Update all zombies
    for (int i = 0; i < _zombies.size(); i++) {
        _zombies[i]->update(_levels[_currentLevel]->getLevelData(),
            _humans,
            _zombies);
    }
    // Update Zombie collisions
    for (int i = 0; i < _zombies.size(); i++) {
        // Collide with other zombies
        for (int j = i + 1; j < _zombies.size(); j++) {
            _zombies[i]->collideWithAgent(_zombies[j]);
        }
        // Collide with humans
        for (int j = 1; j < _humans.size(); j++) {
            if (_zombies[i]->collideWithAgent(_humans[j])) {
                // Add the new zombie
                _zombies.push_back(new Zombie);
                _zombies.back()->init(ZOMBIE_SPEED, _humans[j]->getPosition());
                // Delete the human
                delete _humans[j];
                _humans[j] = _humans.back();
                _humans.pop_back();
            }
        }

        // Collide with player
        if (_zombies[i]->collideWithAgent(_player)) {
            Bengine::fatalError("YOU LOSE");
        }
    }

    // Update Human collisions
    for (int i = 0; i < _humans.size(); i++) {
        // Collide with other humans
        for (int j = i + 1; j < _humans.size(); j++) {
            _humans[i]->collideWithAgent(_humans[j]);
        }
    }


}

void MainGame::updateBullets() {
    for (int i = 0; i < _bullets.size(); i++) {
        _bullets[i].update(_humans, _zombies);
    }
}

void MainGame::processInput() {
    SDL_Event evnt;
    //Will keep looping until there are no more events to process
    while (SDL_PollEvent(&evnt)) {
        switch (evnt.type) {
        case SDL_QUIT:
            // Exit the game here!
            break;
        case SDL_MOUSEMOTION:
            _inputManager.setMouseCoords(evnt.motion.x, evnt.motion.y);
            break;
        case SDL_KEYDOWN:
            _inputManager.pressKey(evnt.key.keysym.sym);
            break;
        case SDL_KEYUP:
            _inputManager.releaseKey(evnt.key.keysym.sym);
            break;
        case SDL_MOUSEBUTTONDOWN:
            _inputManager.pressKey(evnt.button.button);
            break;
        case SDL_MOUSEBUTTONUP:
            _inputManager.releaseKey(evnt.button.button);
            break;
        }
    }
}

void MainGame::drawGame() {
    // Set the base depth to 1.0
    glClearDepth(1.0);
    // Clear the color and depth buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    _textureProgram.use();

    // Draw code goes here
    glActiveTexture(GL_TEXTURE0);

    // Make sure the shader uses texture 0
    GLint textureUniform = _textureProgram.getUniformLocation("mySampler");
    glUniform1i(textureUniform, 0);

    // Grab the camera matrix
    glm::mat4 projectionMatrix = _camera.getCameraMatrix();
    GLint pUniform = _textureProgram.getUniformLocation("P");
    glUniformMatrix4fv(pUniform, 1, GL_FALSE, &projectionMatrix[0][0]);

    // Draw the level
    _levels[_currentLevel]->draw();

    // Begin drawing agents
    _agentSpriteBatch.begin();

    // Draw the humans
    for (int i = 0; i < _humans.size(); i++) {
        _humans[i]->draw(_agentSpriteBatch);
    }

    // Draw the zombies
    for (int i = 0; i < _zombies.size(); i++) {
        _zombies[i]->draw(_agentSpriteBatch);
    }

    // Draw the bullets
    for (int i = 0; i < _bullets.size(); i++) {
        _bullets[i].draw(_agentSpriteBatch);
    }

    _agentSpriteBatch.end();

    _agentSpriteBatch.renderBatch();

    _textureProgram.unuse();

    // Swap our buffer and draw everything to the screen!
    _window.swapBuffer();
}