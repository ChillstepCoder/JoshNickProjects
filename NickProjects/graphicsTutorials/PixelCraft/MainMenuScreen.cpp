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
    return m_screenIndex + 1;
}

int MainMenuScreen::getPreviousScreenIndex() const {
    return m_screenIndex - 1;
}

void MainMenuScreen::build() {
    Bengine::ImGuiManager::init(m_window);

    // Shader.init
    // Compile our color shader
    m_textureProgram.compileShaders("Shaders/textureShadingVert.txt", "Shaders/textureShadingFrag.txt");
    m_textureProgram.addAttribute("vertexPosition");
    m_textureProgram.addAttribute("vertexColor");
    m_textureProgram.addAttribute("vertexUV");
    m_textureProgram.linkShaders();
}

void MainMenuScreen::destroy() {

}

void MainMenuScreen::onEntry() {
    std::cout << "OnEntry\n";
    m_screenIndex = 0;

    m_spriteBatch.init();

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

    if (ImGui::Button("Play")) {
        setState(Bengine::ScreenState::CHANGE_NEXT);
    }

    if (ImGui::Button("Texture Editor")) {
        m_screenIndex = 1;
        setState(Bengine::ScreenState::CHANGE_NEXT);
    }

    ImGui::End();

}