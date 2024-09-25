#pragma once

#include <SDL/SDL.h>
#include <GL/glew.h>

#include <Bengine/Bengine.h>
#include <Bengine/GLSLProgram.h>
#include <Bengine/GLTexture.h>
#include <Bengine/Sprite.h>
#include <Bengine/Window.h>
#include <Bengine/InputManager.h>
#include <Bengine/Timing.h>

#include <Bengine/SpriteBatch.h>

#include <Bengine/Camera2D.h>

#include "Bullet.h"

#include <vector>

enum class GameState {PLAY, EXIT};

class MainGame
{
public:
    MainGame();
    ~MainGame();

    void run();

private:
    void initSystems();
    void initShaders();
    void gameLoop();
    void processInput();
    void drawGame();
    
    Bengine::Window m_window;
    int m_screenWidth;
    int m_screenHeight;
    GameState m_gameState;

    Bengine::GLSLProgram m_colorProgram;
    Bengine::Camera2D m_camera;

    Bengine::SpriteBatch m_spriteBatch;

    Bengine::InputManager m_inputManager;
    Bengine::FpsLimiter m_fpsLimiter;

    std::vector<Bullet> m_bullets;

    float m_maxFPS;
    float m_fps;
    float m_time;
};

