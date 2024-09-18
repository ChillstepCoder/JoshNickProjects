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
    _player(nullptr),
    _numHumansKilled(0),
    _numZombiesKilled(0) {
    // Empty
}

MainGame::~MainGame() {
    for (int i = 0; i < _levels.size(); i++) {
        delete _levels[i];
    }
}

void MainGame::run() {
    initSystems();

    initLevels(); // Initializes all levels

    initLevel(); // Initialize the first level

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

void MainGame::initLevels() {
    _levels.push_back(new Level("Levels/level1.txt"));
    _levels.push_back(new Level("Levels/level2.txt"));
    _levels.push_back(new Level("Levels/level3.txt"));
    _levels.push_back(new Level("Levels/level4.txt"));
    _levels.push_back(new Level("Levels/level5.txt"));
    _levels.push_back(new Level("Levels/level6.txt"));
    // Add more levels as needed
}

void MainGame::initLevel() {
    // Clean up previous level data
    for (auto human : _humans) {
        delete human;
    }
    _humans.clear();

    for (auto zombie : _zombies) {
        delete zombie;
    }
    _zombies.clear();

    _bullets.clear();

    // Load the current level
    if (_currentLevel < _levels.size()) {
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
        _player->addGun(new Gun("Hose of Doom", 1, 100, 0.35f, 40, 30.0f)); // Name, firerate, # of shots, spread, damage, bullet speed
    }
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

        checkVictory();

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
    // Update and collide with world
    for (int i = 0; i < _bullets.size();) {
        // If update returns true, the bullet collided with a wall
        if (_bullets[i].update(_levels[_currentLevel]->getLevelData())) {
            _bullets[i] = _bullets.back();
            _bullets.pop_back();
        } else {
            i++;
        }
    }
    
    bool wasBulletRemoved;

    // Collide with humans and zombies
    for (int i = 0; i < _bullets.size(); i++) {
        wasBulletRemoved = false;
        // Loop through zombies
        for (int j = 0; j < _zombies.size(); ) {
            // Check collision
            if (_bullets[i].collideWithAgent(_zombies[j])) {
                // Damage zombie, and kill it if its out of health
                if (_zombies[j]->applyDamage(_bullets[i].getDamage())) {
                    // If the zombie died, remove him
                    delete _zombies[j];
                    _zombies[j] = _zombies.back();
                    _zombies.pop_back();
                    _numZombiesKilled++;
                } else {
                    j++;
                }

                // Remove the bullet
                _bullets[i] = _bullets.back();
                _bullets.pop_back();
                wasBulletRemoved = true;
                i--; //< Make sure we dont skip a bullet
                // Since the bullet died, no need to loop through any more zombies
                break;
            } else {
                j++;
            }
        }
        // Loop through humans
        if (wasBulletRemoved == false) {
            for (int j = 1; j < _humans.size(); ) {
                // Check collision
                if (_bullets[i].collideWithAgent(_humans[j])) {
                    // Damage human, and kill it if its out of health
                    if (_humans[j]->applyDamage(_bullets[i].getDamage())) {
                        // If the human died, remove him
                        delete _humans[j];
                        _humans[j] = _humans.back();
                        _humans.pop_back();
                        _numHumansKilled++;
                    }
                    else {
                        j++;
                    }

                    // Remove the bullet
                    _bullets[i] = _bullets.back();
                    _bullets.pop_back();
                    i--; //< Make sure we dont skip a bullet
                    // Since the bullet died, no need to loop through any more zombies
                    break;
                }
                else {
                    j++;
                }
            }
        }

    }
}

void MainGame::checkVictory() {
    if (_zombies.empty()) {

        std::printf("*** You win! ***\n You killed %d humans and %d zombies. There are %d/%d humans remaining."
            , _numHumansKilled, _numZombiesKilled, _humans.size() - 1, _levels[_currentLevel]->getNumHumans());

        // Move to the next level
        _currentLevel++;
        if (_currentLevel < _levels.size()) {
            initLevel(); //Initialize the new level
        }
        else {
            //when all levels are complete
            Bengine::fatalError("All levels completed! Game over.");
        }
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