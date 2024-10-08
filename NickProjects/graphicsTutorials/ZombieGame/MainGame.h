#pragma once

#include <Bengine/Window.h>
#include <Bengine/GLSLProgram.h>
#include <Bengine/Camera2D.h>
#include <Bengine/InputManager.h>
#include <Bengine/SpriteBatch.h>
#include <Bengine/SpriteFont.h>
#include <Bengine/AudioEngine.h>
#include <Bengine/ParticleEngine2D.h>
#include <Bengine/ParticleBatch2D.h>
#include <memory>

#include "Player.h"
#include "Level.h"
#include "Bullet.h"

class Zombie;

enum class GameState { 
    PLAY,
    EXIT
};

class MainGame
{
public:
    MainGame();
    ~MainGame();

    // Runs the game
    void run();

private:
    // Initializes the core systems
    void initSystems();

    // Initializes all levels
    void initLevels();

    // Initializes the level and sets up everything
    void initNextLevel();

    // Initializes the shaders
    void initShaders();

    // Main game loop for the program
    void gameLoop();

    // Updates all agents
    void updateAgents(float deltaTime);

    // Update all bullets
    void updateBullets(float deltaTime);

    // Checks the victory condition
    void checkVictory();

    // Handles input processing
    void processInput();

    // Renders the game
    void drawGame();

    // Draws the HUD
    void drawHud();

    // Adds blood to the particle engine
    void addBlood(const glm::vec2& position, int numParticles);

    // Member Variables
    Bengine::Window _window; //< The game window

    Bengine::GLSLProgram _textureProgram; //< The shader program
    Bengine::GLSLProgram _textRenderingProgram; //< The text shader program

    Bengine::InputManager _inputManager; //< Handles input

    Bengine::Camera2D _camera; //< Main Camera
    Bengine::Camera2D _hudCamera; //< Hud Camera

    Bengine::SpriteBatch _agentSpriteBatch; //< Draws all agents
    Bengine::SpriteBatch _hudSpriteBatch;

    Bengine::ParticleEngine2D m_particleEngine;
    Bengine::ParticleBatch2D* m_bloodParticleBatch;

    std::vector<Level*> _levels; //< Vector of all levels

    int _screenWidth, _screenHeight;

    float _fps;

    int _currentLevel;

    Player* _player;
    std::vector<Human*> _humans; //< Vector of all humans
    std::vector<Zombie*> _zombies; //< Vector of all zombies
    std::vector<Bullet> _bullets;

    int _numHumansKilled; //< Humans killed by player
    int _numZombiesKilled; //< Zombies killed by player

    std::unique_ptr<Bengine::SpriteFont> _spriteFont;

    Bengine::AudioEngine m_audioEngine;

    GameState _gameState;
};