#include "TextureEditorScreen.h"
#include <iostream>
#include <SDL/SDL.h>
#include <Bengine/IMainGame.h>
#include <Bengine/ScreenList.h>
#include "Bengine/ImGuiManager.h"
#include <Bengine/ResourceManager.h>
#include "Block.h"

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
    // Shader.init
    // Compile our color shader
    m_textureProgram.compileShaders("Shaders/textureShadingVert.txt", "Shaders/textureShadingFrag.txt");
    m_textureProgram.addAttribute("vertexPosition");
    m_textureProgram.addAttribute("vertexColor");
    m_textureProgram.addAttribute("vertexUV");
    m_textureProgram.linkShaders();
}

void TextureEditorScreen::destroy() {

}

void TextureEditorScreen::onEntry() {
    std::cout << "OnEntry\n";

    m_spriteBatch.init();

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
    ImGui::SetNextWindowPos(ImVec2(520, 510), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(200, 150), ImGuiCond_FirstUseEver);

    ImGui::Begin("Texture Editor");
    GLuint textureID = Bengine::ResourceManager::getTexture("Textures/Stone.png").id;
    BlockDefRepository repository;
    BlockID id = BlockID::STONE;

    ImVec2 display_min = ImVec2(0.0f, 0.0f);
    ImVec2 display_size = ImVec2(450.0f, 450.0f);
    ImVec2 texture_size = ImVec2(288.0f, 270.0f);

    ImVec2 uv0 = ImVec2(display_min.x / texture_size.x, display_min.y / texture_size.y);

    ImVec2 uv1 = ImVec2((display_min.x + display_size.x) / texture_size.x, (display_min.y + display_size.y) / texture_size.y);

    ImGui::Text("uv0 = (%f, %f)", uv0.x, uv0.y);
    ImGui::Text("uv1 = (%f, %f)", uv1.x, uv1.y);

    //glm::vec4 uvRect = blockDef.getSubUVRect(glm::ivec2(3, 3), TILE_ATLAS_DIMS_CELLS);

    //glm::vec4 uvRectFixed = glm::vec4(uvRect.x, uvRect.y += pixelHeight, uvRect.z -= pixelWidth, uvRect.w -= pixelHeight);


    if (ImGui::Button("Previous")) {

    }
    ImGui::SameLine();
    if (ImGui::Button("Next")) {

    }
    if (ImGui::Button("Return to Menu")) {
        m_screenIndex = 1;
        setState(Bengine::ScreenState::CHANGE_PREVIOUS);
    }

    ImGui::End();

    ImGui::SetNextWindowPos(ImVec2(520, 10), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(200, 150), ImGuiCond_FirstUseEver);
    ImGui::Begin("Texture", nullptr, ImGuiWindowFlags_NoCollapse);

    ImGui::Image((ImTextureID)std::uintptr_t(textureID), ImVec2(display_size.x, display_size.y), uv0, uv1);

    ImGui::End();
}