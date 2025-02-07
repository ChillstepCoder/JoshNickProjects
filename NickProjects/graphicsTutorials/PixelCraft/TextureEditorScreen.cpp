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

    BlockDefRepository::initBlockDefs();

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
    ImGui::SetNextWindowSize(ImVec2(200, 300), ImGuiCond_FirstUseEver);

    ImGui::Begin("Texture Editor");
    GLuint textureID = Bengine::ResourceManager::getTexture("Textures/Stone.png", Bengine::TextureFilterMode::Nearest).id;
    BlockDefRepository repository;
    BlockID id = BlockID::STONE;
    BlockDef blockDef = repository.getDef(id);

    ImVec2 display_size = ImVec2(600.0f, 600.0f);

    static int subUV_X = 0;
    static int subUV_Y = 0;

    glm::vec4 uvRect = blockDef.getSubUVRect(glm::ivec2(subUV_X, subUV_Y), TILE_ATLAS_DIMS_CELLS);

    float pixelWidth = 0.00694f;
    float pixelHeight = 0.00740f;

    glm::vec4 uvRectFixed = glm::vec4(uvRect.x, uvRect.y += pixelHeight, uvRect.z -= pixelWidth, uvRect.w -= pixelHeight); // need this because the .png is slightly incorrect


    if (ImGui::Button("Previous")) {
        if (subUV_X > 0) {
            subUV_X--;
        }
        else if (subUV_Y > 0) {
            subUV_Y--;
            subUV_X = TILE_ATLAS_DIMS_CELLS.x;
        }

    }
    ImGui::SameLine();
    if (ImGui::Button("Next")) {
        if (subUV_X < TILE_ATLAS_DIMS_CELLS.x) {
            subUV_X++;
        }
        else if (subUV_Y < TILE_ATLAS_DIMS_CELLS.y) {
            subUV_Y++;
            subUV_X = 1;
        }

    }
    if (ImGui::Button("Return to Menu")) {
        m_screenIndex = 1;
        setState(Bengine::ScreenState::CHANGE_PREVIOUS);
    }


    ImGui::Text("SubUV (x,y) = (%d, %d)", subUV_X, subUV_Y);
    ImGui::Text("uv0 = (%f, %f)", uvRectFixed.x, uvRectFixed.y);
    ImGui::Text("uv1 = (%f, %f)", uvRectFixed.x + uvRectFixed.z, uvRectFixed.y + uvRectFixed.w);

    ImGui::End();

    ImGui::SetNextWindowPos(ImVec2(520, 0), ImGuiCond_Appearing);
    ImGui::SetNextWindowSize(ImVec2(display_size.x * 2, display_size.y), ImGuiCond_Appearing);
    ImGui::Begin("Texture", nullptr, ImGuiWindowFlags_NoCollapse);

    ImGui::Image((ImTextureID)std::uintptr_t(textureID), ImVec2(display_size.x, display_size.y), ImVec2(uvRectFixed.x, uvRectFixed.y), ImVec2(uvRectFixed.x + uvRectFixed.z, uvRectFixed.y + uvRectFixed.w));

    // Full texture for reference
    ImGui::SameLine();
    ImGui::Image((ImTextureID)std::uintptr_t(textureID), ImVec2(display_size.x, display_size.y), ImVec2(0, 0), ImVec2(1, 1));

    ImGui::End();
}