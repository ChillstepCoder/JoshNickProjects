#pragma once

#include <Bengine/Camera2D.h>
#include <Bengine/SpriteBatch.h>
#include <Bengine/InputManager.h>
#include <Bengine/Window.h>
#include <Bengine/GLSLProgram.h>
#include <Bengine/Timing.h>
#include <Bengine/SpriteFont.h>
#include <memory>
#include "imgui.h"
#include "ImGui/backends/imgui_impl_sdl2.h"
#include "ImGui/backends/imgui_impl_opengl3.h"

#include "BallController.h"
#include "BallRenderer.h"
#include "Grid.h"

// TODO:
// Visualize momentum with color
// Visualize velocity with color
// Visualize position with color

enum class GameState { RUNNING, EXIT };

const int CELL_SIZE = 12;

class MainGame {
public:
    ~MainGame();
    void run();


private:
    void init();
    void initRenderers();
    void initBalls(int numBalls, float ballSize);
    void update(float deltaTime);
    void draw();
    void drawHud();
    void drawImgui();
    void processInput();

    int m_screenWidth = 0;
    int m_screenHeight = 0;
    int m_currentShader = 0;
    glm::vec3 m_shaderColor = glm::vec3(1.0f, 1.0f, 1.0f);
    void applyShaderChanges();
    void resetGame(int newNumBalls, float newBallSize);

    std::vector<Ball> m_balls; ///< All the balls
    std::unique_ptr<Grid> m_grid; ///< Grid for spatial partitioning for collision

    int m_currentRenderer = 0;
    std::vector<std::unique_ptr<BallRenderer> > m_ballRenderers;

    BallController m_ballController; ///< Controls balls

    Bengine::Window m_window; ///< The main window
    Bengine::SpriteBatch m_spriteBatch; ///< Renders all the balls
    std::unique_ptr<Bengine::SpriteFont> m_spriteFont; ///< For font rendering
    Bengine::Camera2D m_camera; ///< Renders the scene
    Bengine::InputManager m_inputManager; ///< Handles input
    Bengine::GLSLProgram m_textureProgram; ///< Shader for textures]
    Bengine::GLSLProgram m_textRenderingProgram; ///< Shader for text]
    Bengine::FpsLimiter m_fpsLimiter; ///< Limits and calculates fps
    float m_fps = 0.0f;

    GameState m_gameState = GameState::RUNNING; ///< The state of the game
};