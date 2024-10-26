#pragma once

#include "Box.h"
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

private:
    void checkInput();

    Bengine::SpriteBatch m_spriteBatch;
    Bengine::GLSLProgram m_textureProgram;
    Bengine::Camera2D m_camera;
    Bengine::GLTexture m_texture;
    Bengine::Window* m_window;

    Player m_player;
    std::vector<Box> m_boxes;
    b2WorldId m_world = b2_nullWorldId;
    b2BodyId m_ground = b2_nullBodyId;

    DebugDraw m_debugDraw;
};