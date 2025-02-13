#pragma once

#include <Bengine/IGameScreen.h>
#include <Bengine/SpriteBatch.h>
#include <Bengine/GLSLProgram.h>
#include <Bengine/Camera2D.h>
#include <Bengine/GLTexture.h>
#include <Bengine/Window.h>
#include <Bengine/Timing.h>
#include <Bengine/SpriteFont.h>
#include <Bengine/DebugOpenGL.h>
#include "Imgui.h"
#include "ImGui/backends/imgui_impl_sdl2.h"
#include "ImGui/backends/imgui_impl_opengl3.h"

class ConnectedTextureSet;

class TextureEditorScreen : public Bengine::IGameScreen {
public:
    TextureEditorScreen(Bengine::Window* window);
    ~TextureEditorScreen();

    virtual int getNextScreenIndex() const override;

    virtual int getPreviousScreenIndex() const override;

    virtual void build() override;

    virtual void destroy() override;

    virtual void onEntry() override;

    virtual void onExit() override;

    virtual void update() override;

    virtual void draw() override;

    void drawBackground();

    void drawImgui();

    void setScreenIndex(int index) { m_screenIndex = index; }


private:
    void checkInput();

    void updateCurrentSubtexture();
    // Direction represents
    // the nearby cells
    // 5 6 7
    // 3   4
    // 0 1 2
    void renderRuleDropDown(int direction);

    Bengine::SpriteBatch m_spriteBatch;
    Bengine::Camera2D m_camera;
    Bengine::GLSLProgram m_textureProgram;
    GLuint m_background;
    Bengine::Window* m_window;
    ConnectedTextureSet& m_connectedTextureSet;
    int subUV_X = 0;
    int subUV_Y = 0;
    int m_screenIndex = 2;
};

