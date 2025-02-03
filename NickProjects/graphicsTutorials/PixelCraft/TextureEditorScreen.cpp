#include "TextureEditorScreen.h"
#include <iostream>
#include <SDL/SDL.h>
#include <Bengine/IMainGame.h>
#include <Bengine/ScreenList.h>
#include "Bengine/ImGuiManager.h"


TextureEditorScreen::TextureEditorScreen(Bengine::Window* window) : m_window(window) {

}
TextureEditorScreen::~TextureEditorScreen() {

}

int TextureEditorScreen::getNextScreenIndex() const {
    return m_screenIndex + 1;
}

int TextureEditorScreen::getPreviousScreenIndex() const {
    return m_screenIndex - 1;
}

void TextureEditorScreen::build() {

}

void TextureEditorScreen::destroy() {

}

void TextureEditorScreen::onEntry() {
    std::cout << "OnEntry\n";

    m_spriteBatch.init();

    Bengine::ImGuiManager::init(m_window);

    // Shader.init
    // Compile our color shader
    m_textureProgram.compileShaders("Shaders/textureShadingVert.txt", "Shaders/textureShadingFrag.txt");
    m_textureProgram.addAttribute("vertexPosition");
    m_textureProgram.addAttribute("vertexColor");
    m_textureProgram.addAttribute("vertexUV");
    m_textureProgram.linkShaders();

    m_camera.init(m_window->getScreenWidth(), m_window->getScreenHeight());
    m_camera.setScale(20.0f); // 20.0f

}

void TextureEditorScreen::onExit() {



}

void TextureEditorScreen::update() {
    std::cout << "Update\n";
    checkInput();
}

void TextureEditorScreen::draw() {

    if (m_screenIndex != 0) {
        onExit();
    }


    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(0.4f, 0.4f, 0.8f, 1.0f);

    m_textureProgram.use();

    // Upload texture uniform
    GLint textureUniform = m_textureProgram.getUniformLocation("mySampler");
    glUniform1i(textureUniform, 0);
    glActiveTexture(GL_TEXTURE0);

    drawBackground();

    Bengine::ImGuiManager::newFrame();

    drawImgui();

    Bengine::ImGuiManager::renderFrame();

    m_textureProgram.unuse();

}

void TextureEditorScreen::checkInput() {
    SDL_Event evnt;
    while (SDL_PollEvent(&evnt)) {
        Bengine::ImGuiManager::processEvent(evnt);
        m_game->onSDLEvent(evnt);
    }
}

void TextureEditorScreen::drawBackground() {
    m_spriteBatch.begin();


    //m_spriteBatch.draw(m_background);

    m_spriteBatch.end();
    m_spriteBatch.renderBatch();
}


void TextureEditorScreen::drawImgui() {
    ImGui::Begin("Texture Editor");

    static int edit = 0;
    if (ImGui::Button("Edit Textures"))
        edit++;

    if (edit & 1)
    {
        ImGui::SameLine();
        ImGui::Text("Editing!!!!");
    }
    static int texture = 0;
    if (ImGui::Button("Texture stuff"))
        texture++;
    if (texture & 1)
    {
        ImGui::SameLine();
        ImGui::Text("TEXTURINGGGGG!!!");
    }
    int menu = 0;
    if (ImGui::Button("Return to Menu"))
        menu++;
    if (menu & 1)
    {
        m_screenIndex = 1;
        m_game->getCurrentScreen()->setState(Bengine::ScreenState::CHANGE_PREVIOUS);
    }

    ImGui::End();

}