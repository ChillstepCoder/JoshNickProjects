// MainGame.h

#pragma once
#include <JAGEngine/SpriteBatch.h>
#include <JAGEngine/InputManager.h>
#include <JAGEngine/Window.h>
#include <JAGEngine/GLSLProgram.h>
#include <JAGEngine/Timing.h>
#include <JAGEngine/SpriteFont.h>
#include <memory>
#include <JAGEngine/ImGuiManager.h>
#include "BallController.h"
#include "BallRenderer.h"
#include "BallGameCamera.h"
#include "Grid.h"

// TODO:
// Visualize momentum with color
// Visualize velocity with color
// Visualize position with color

enum class GameState { RUNNING, EXIT };

const int CELL_SIZE = 12;

class MainGame {
public:
  enum class GameState {
    RUNNING,
    EXIT
  };

    ~MainGame();
    void run();
    void update(float deltaTime);

private:
    void init();
    void initRenderers();
    void initBalls();
    
    void draw();
    void drawHud();
    void processInput();
    void updateImGui();

    int m_screenWidth = 0;
    int m_screenHeight = 0;

    std::vector<Ball> m_balls; ///< All the balls
    std::unique_ptr<Grid> m_grid; ///< Grid for spatial partitioning for collision

    int m_currentRenderer = 0;
    std::vector<std::unique_ptr<BallRenderer> > m_ballRenderers;

    BallController m_ballController; ///< Controls balls

    JAGEngine::Window m_window; ///< The main window
    JAGEngine::SpriteBatch m_spriteBatch; ///< Renders all the balls
    std::unique_ptr<JAGEngine::SpriteFont> m_spriteFont; ///< For font rendering
    BallGameCamera m_camera;
    JAGEngine::InputManager m_inputManager; ///< Handles input
    JAGEngine::GLSLProgram m_textureProgram; ///< Shader for textures]

    JAGEngine::FpsLimiter m_fpsLimiter; ///< Limits and calculates fps
    float m_fps = 0.0f;
    float m_hueShift = 0.0f;

    GameState m_gameState = GameState::RUNNING; ///< The state of the game
};

