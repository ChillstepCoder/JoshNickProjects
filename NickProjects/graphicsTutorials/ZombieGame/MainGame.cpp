#include "MainGame.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <Bengine/Bengine.h>
#include <Bengine/Timing.h>
#include <random>
#include <ctime>
#include <Bengine/BengineErrors.h>
#include <Bengine/ResourceManager.h>

#include <SDL/SDL.h>
#include <iostream>
#include <glm/gtx/rotate_vector.hpp>

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

    Bengine::Music music = m_audioEngine.loadMusic("Sound/Infested City.ogg");
    music.play(-1);

    initNextLevel(); // Initialize the first level

    gameLoop();

}

void MainGame::initSystems() {
    Bengine::init();

    // Initialize sound, must happen after Bengine::init
    m_audioEngine.init();

    _window.create("ZombieGame", _screenWidth, _screenHeight, 0);
    glClearColor(0.7f, 0.7f, 0.7f, 1.0f);

    initShaders();

    _agentSpriteBatch.init();
    _hudSpriteBatch.init();

    // init Font
    _spriteFont = std::make_unique<Bengine::SpriteFont>();
    _spriteFont->init("Fonts/Bubblefont.ttf", 32);

    // Set up the camera
    _camera.init(_screenWidth, _screenHeight);
    _hudCamera.init(_screenWidth, _screenHeight);
    _hudCamera.setPosition(glm::vec2(_screenWidth / 2, _screenHeight / 2));

    // Initialize particles
    m_bloodParticleBatch = new Bengine::ParticleBatch2D();
    m_bloodParticleBatch->init(1000, 0.05f, Bengine::ResourceManager::getTexture("Textures/Particles/blood.png"));
    m_particleEngine.addParticleBatch(m_bloodParticleBatch);

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

void MainGame::initNextLevel() {
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
        _player->addGun(new Gun("Magnum", 15, 1, 0.1f, 30, BULLET_SPEED, m_audioEngine.loadSoundEffect("Sound/lmg_fire01.mp3")));
        _player->addGun(new Gun("Shotgun", 35, 20, 0.4f, 4, BULLET_SPEED, m_audioEngine.loadSoundEffect("Sound/doomshotgun.mp3")));
        _player->addGun(new Gun("AK-47", 4, 1, 0.15f, 20, BULLET_SPEED, m_audioEngine.loadSoundEffect("Sound/lmg_fire01.mp3")));
        _player->addGun(new Gun("Hose of Doom", 1, 100, 0.35f, 40, 30.0f, m_audioEngine.loadSoundEffect("Sound/lmg_fire01.mp3"))); // Name, firerate, # of shots, spread, damage, bullet speed
    }
}

void MainGame::initShaders() {
    // Compile our color shader
    _textureProgram.compileShaders("Shaders/textureShadingVert.txt", "Shaders/textureShadingFrag.txt");
    _textureProgram.addAttribute("vertexPosition");
    _textureProgram.addAttribute("vertexColor");
    _textureProgram.addAttribute("vertexUV");
    _textureProgram.linkShaders();

    _textRenderingProgram.compileShaders("Shaders/textRenderingVert.txt", "Shaders/textRenderingFrag.txt");
    _textRenderingProgram.addAttribute("vertexPosition");
    _textRenderingProgram.addAttribute("vertexColor");
    _textRenderingProgram.addAttribute("vertexUV");
    _textRenderingProgram.linkShaders();
}

void MainGame::gameLoop() {
    
    const float DESIRED_FPS = 60.0f;
    const int MAX_PHYSICS_STEPS = 6;

    Bengine::FpsLimiter fpsLimiter;
    fpsLimiter.setMaxFPS(144.0f);

    const float CAMERA_SCALE = 1.0f / 2.0f;
    _camera.setScale(CAMERA_SCALE);

    const float MS_PER_SECOND = 1000.0f;
    const float DESIRED_FRAMETIME = MS_PER_SECOND / DESIRED_FPS;
    const float MAX_DELTA_TIME = 1.0f;

    float previousTicks = SDL_GetTicks();

    while (_gameState == GameState::PLAY) {
        fpsLimiter.begin();

        float newTicks = SDL_GetTicks();
        float frameTime = newTicks - previousTicks;
        previousTicks = newTicks;
        float totalDeltaTime = frameTime / DESIRED_FRAMETIME;

        checkVictory();

        _inputManager.update();

        processInput();

        int i = 0;
        while (totalDeltaTime > 0.0f && i < MAX_PHYSICS_STEPS) {
            float deltaTime = std::min(totalDeltaTime, MAX_DELTA_TIME);
            updateAgents(deltaTime);
            updateBullets(deltaTime);
            m_particleEngine.update(deltaTime);
            totalDeltaTime -= deltaTime;
            i++;
        }

        // Make sure the camera is bound to the player position
        _camera.setPosition(_player->getPosition());
        _camera.update();
        _hudCamera.update();

        drawGame();

        _fps = fpsLimiter.end();
        std::cout << _fps << std::endl;
    }
}

void MainGame::updateAgents(float deltaTime) {
    // Update all humans
    for (int i = 0; i < _humans.size(); i++) {
        _humans[i]->update(_levels[_currentLevel]->getLevelData(),
                           _humans,
                           _zombies,
                           deltaTime);
    }

    // Update all zombies
    for (int i = 0; i < _zombies.size(); i++) {
        _zombies[i]->update(_levels[_currentLevel]->getLevelData(),
            _humans,
            _zombies,
            deltaTime);
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

void MainGame::updateBullets(float deltaTime) {
    // Update and collide with world
    for (int i = 0; i < _bullets.size();) {
        // If update returns true, the bullet collided with a wall
        if (_bullets[i].update(_levels[_currentLevel]->getLevelData(), deltaTime)) {
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
                // Add blood
                addBlood(_bullets[i].getPosition(), 5);
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
                    // Add blood
                    addBlood(_bullets[i].getPosition(), 5);
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
            initNextLevel(); //Initialize the new level
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

    // Render the particles
    m_particleEngine.draw(&_agentSpriteBatch);

    const glm::vec2 agentDims(AGENT_RADIUS * 2.0f);

    // Draw the humans
    for (int i = 0; i < _humans.size(); i++) {
        if (_camera.isBoxInView(_humans[i]->getPosition(), agentDims)) {
            _humans[i]->draw(_agentSpriteBatch);
        }
    }

    // Draw the zombies
    for (int i = 0; i < _zombies.size(); i++) {
        if (_camera.isBoxInView(_zombies[i]->getPosition(), agentDims)) {
            _zombies[i]->draw(_agentSpriteBatch);
        }
    }

    // Draw the bullets
    for (int i = 0; i < _bullets.size(); i++) {
        if (_camera.isBoxInView(_bullets[i].getPosition(), agentDims)) {
            _bullets[i].draw(_agentSpriteBatch);
        }
    }

    _agentSpriteBatch.end();

    _agentSpriteBatch.renderBatch();

    // Render the heads up display
    drawHud();

    _textureProgram.unuse();

    // Swap our buffer and draw everything to the screen!
    _window.swapBuffer();
}

void MainGame::drawHud() {
    char buffer[256];

    glm::mat4 projectionMatrix = _hudCamera.getCameraMatrix();
    GLint pUniform = _textureProgram.getUniformLocation("P");
    glUniformMatrix4fv(pUniform, 1, GL_FALSE, &projectionMatrix[0][0]);

    _hudSpriteBatch.begin();


    sprintf_s(buffer, "Num Humans %d", _humans.size());
    _spriteFont->draw(_hudSpriteBatch, buffer, glm::vec2(0, 0),
                      glm::vec2(0.5), 0.0f, Bengine::ColorRGBA8(0, 0, 255, 255));


    sprintf_s(buffer, "Num Zombies %d", _zombies.size());
    _spriteFont->draw(_hudSpriteBatch, buffer, glm::vec2(0, 36),
        glm::vec2(0.5), 0.0f, Bengine::ColorRGBA8(255, 0, 0, 255));

    _hudSpriteBatch.end();
    _hudSpriteBatch.renderBatch();
}

void MainGame::addBlood(const glm::vec2& position, int numParticles) {

    static std::mt19937 randEngine(time(nullptr));
    static std::uniform_real_distribution<float> randAngle(0.0f, 2 * M_PI);

    glm::vec2 vel(1.15f, 0.0f);
    Bengine::ColorRGBA8 col(255, 0, 0, 255);

    for (int i = 0; i < numParticles; i++) {
        m_bloodParticleBatch->addParticle(position, glm::rotate(vel, randAngle(randEngine)), col, 25.0f);
    }
}