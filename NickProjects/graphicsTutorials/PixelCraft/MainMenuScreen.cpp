#include "MainMenuScreen.h"
#include <iostream>
#include <SDL/SDL.h>
#include <Bengine/IMainGame.h>
#include <Bengine/ScreenList.h>
#include "Bengine/ImGuiManager.h"


MainMenuScreen::MainMenuScreen(Bengine::Window* window) : m_window(window) {

}
MainMenuScreen::~MainMenuScreen() {

}

int MainMenuScreen::getNextScreenIndex() const {
    return screenIndex + 1;
}

int MainMenuScreen::getPreviousScreenIndex() const {
    return screenIndex - 1;
}

void MainMenuScreen::build() {

}

void MainMenuScreen::destroy() {

}

void MainMenuScreen::onEntry() {
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

void MainMenuScreen::onExit() {



}

void MainMenuScreen::update() {
    std::cout << "Update\n";
    checkInput();
}

void MainMenuScreen::draw() {

    if (screenIndex != 0) {
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

void MainMenuScreen::checkInput() {
    SDL_Event evnt;
    while (SDL_PollEvent(&evnt)) {
        Bengine::ImGuiManager::processEvent(evnt);
        m_game->onSDLEvent(evnt);
    }
}

void MainMenuScreen::drawBackground() {
    m_spriteBatch.begin();


    //m_spriteBatch.draw(m_background);

    m_spriteBatch.end();
    m_spriteBatch.renderBatch();
}


void MainMenuScreen::drawImgui() {
    ImGui::Begin("Main Menu");

    static int play = 0;
    if (ImGui::Button("Play"))
        play++;

    if (play & 1)
    {
        m_game->getCurrentScreen()->setState(Bengine::ScreenState::CHANGE_NEXT);

        ImGui::SameLine();
        ImGui::Text("Playing!!!!");
    }
    static int texture = 0;
    if (ImGui::Button("Texture Editor"))
        texture++;
    if (texture & 1)
    {
        ImGui::SameLine();
        ImGui::Text("TEXTURINGGGGG!!!");
    }

    ImGui::End();

}