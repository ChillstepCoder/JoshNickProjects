#pragma once

#include "Block.h"
#include "Player.h"
#include <Box2D/box2d.h>
#include <Bengine/IGameScreen.h>
#include <vector>
#include <memory>
#include <Bengine/SpriteBatch.h>
#include <Bengine/GLSLProgram.h>
#include <Bengine/Camera2D.h>
#include <Bengine/GLTexture.h>
#include <Bengine/Window.h>
#include <Bengine/Timing.h>
#include <Bengine/SpriteFont.h>
#include "BlockMeshManager.h"
#include "DebugDraw.h"
#include "Imgui.h"
#include "ImGui/backends/imgui_impl_sdl2.h"
#include "ImGui/backends/imgui_impl_opengl3.h"

class BlockManager;

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

    void drawHud();

    Bengine::SpriteBatch m_spriteBatch;
    Bengine::GLSLProgram m_textureProgram;
    Bengine::GLSLProgram m_textRenderingProgram; ///< Shader for text
    Bengine::Camera2D m_camera;
    Bengine::GLTexture m_texture;
    Bengine::Window* m_window;
    std::unique_ptr<Bengine::SpriteFont> m_spriteFont; ///< For font rendering

    Player m_player;
    b2WorldId m_world = b2_nullWorldId;
    b2BodyId m_ground = b2_nullBodyId;

    bool m_debugRenderEnabled = false;
    float m_debugAlpha = 0.5f; // Transparency value for debug rendering
    float m_gravity = -80.0f;

    BlockMeshManager m_blockMeshManager;
    BlockManager* m_blockManager;
};