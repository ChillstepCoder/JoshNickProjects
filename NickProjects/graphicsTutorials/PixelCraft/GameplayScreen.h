#pragma once

#include "Block.h"
#include "Player.h"
#include <Box2D/box2d.h>
#include <Bengine/IGameScreen.h>
#include <vector>
#include <Bengine/SpriteBatch.h>
#include <Bengine/GLSLProgram.h>
#include <Bengine/Camera2D.h>
#include <Bengine/GLTexture.h>
#include <Bengine/Window.h>
#include "DebugDraw.h"
#include "Imgui.h"
#include "ImGui/backends/imgui_impl_sdl2.h"
#include "ImGui/backends/imgui_impl_opengl3.h"

class GameplayScreen : public Bengine::IGameScreen {
public:
    GameplayScreen(Bengine::Window* window);
    ~GameplayScreen();

    virtual int getNextScreenIndex() const override;

    virtual int getPreviousScreenIndex() const override;

    virtual void build() override;

    virtual void destroy() override;

    virtual void onEntry() override;

    virtual void onExit() override;

    virtual void update() override;

    virtual void draw() override;

    void drawImgui();

    void updateGravity();

private:
    void checkInput();

    void perlinNoise();

    Bengine::SpriteBatch m_spriteBatch;
    Bengine::GLSLProgram m_textureProgram;
    Bengine::Camera2D m_camera;
    Bengine::GLTexture m_texture;
    Bengine::Window* m_window;

    Player m_player;
    std::vector<Block> m_blocks;
    b2WorldId m_world = b2_nullWorldId;
    b2BodyId m_ground = b2_nullBodyId;

    DebugDraw m_debugDraw;
    bool m_debugRenderEnabled = false;
    float m_debugAlpha = 0.5f; // Transparency value for debug rendering
    float m_gravity = -180.0f;
};