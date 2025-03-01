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
#include <Bengine/DebugOpenGL.h>
#include "BlockMeshManager.h"
#include "CellularAutomataManager.h"
#include "DebugDraw.h"
#include "Imgui.h"
#include "ImGui/backends/imgui_impl_sdl2.h"
#include "ImGui/backends/imgui_impl_opengl3.h"

class BlockManager;
class ConnectedTextureSet;

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

    void setGravity(float gravity) { m_gravity = gravity; }

    bool m_debugRenderEnabled = false;
    std::unordered_map<std::string, float> m_maxTimes;

    float m_caveScale = 0.006f;
    float m_baseCaveThreshold = 0.20f;
    float m_detailScale = 0.03f;
    float m_detailInfluence = 0.13f;
    float m_minCaveDepth = 12.0f;
    float m_surfaceZone = 100.0f;
    float m_deepZone = 600.0f;
    float m_maxSurfaceBonus = 0.02f;
    float m_maxDepthPenalty = 0.01f;


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
    int m_screenIndex = 1;

    Player m_player;
    b2WorldId m_world = b2_nullWorldId;
    b2BodyId m_ground = b2_nullBodyId;

    float m_debugAlpha = 0.5f; // Transparency value for debug rendering
    float m_gravity = -80.0f;
    int m_updateFrame = 0;


    ConnectedTextureSet& m_connectedTextureSet;
    CellularAutomataManager m_cellularAutomataManager;
    BlockMeshManager m_blockMeshManager;
    BlockManager* m_blockManager;
};