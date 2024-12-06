#include "GameplayScreen.h"
#include <iostream>
#include <SDL/SDL.h>
#include <Bengine/IMainGame.h>
#include <Bengine/ResourceManager.h>
#include "Bengine/ImGuiManager.h"
#include "PerlinNoise.hpp"
#include "Timer.h"
#include "Profiler.h"

GameplayScreen::GameplayScreen(Bengine::Window* window) : m_window(window) {

}
GameplayScreen::~GameplayScreen() {
    delete m_blockManager;
    b2DestroyWorld(m_world);
}

int GameplayScreen::getNextScreenIndex() const {
    return SCREEN_INDEX_NO_SCREEN;
}

int GameplayScreen::getPreviousScreenIndex() const {
    return SCREEN_INDEX_NO_SCREEN;
}

void GameplayScreen::build() {

}

void GameplayScreen::destroy() {

}

void GameplayScreen::onEntry() {
    glm::vec2 playerPos(1024.0f, 400.0f);

    b2WorldDef worldDef = b2DefaultWorldDef();
    worldDef.gravity = b2Vec2(0.0f, m_gravity);
    m_world = b2CreateWorld(&worldDef);

    BlockDefRepository::initBlockDefs();

    m_spriteBatch.init();

    // Initialize BlockMeshManager
    m_blockMeshManager.init();

    m_cellularAutomataManager.init();

    m_blockManager = new BlockManager(m_blockMeshManager, m_world, m_cellularAutomataManager);

    m_blockManager->initializeChunks(playerPos);

    // Init Imgui
    Bengine::ImGuiManager::init(m_window);

    DebugDraw::getInstance().init();

    m_spriteFont->init("Fonts/Chintzy.ttf", 32);

    // Shader.init
    // Compile our color shader
    m_textureProgram.compileShaders("Shaders/textureShadingVert.txt", "Shaders/textureShadingFrag.txt");
    m_textureProgram.addAttribute("vertexPosition");
    m_textureProgram.addAttribute("vertexColor");
    m_textureProgram.addAttribute("vertexUV");
    m_textureProgram.linkShaders();
    // Compile our text shader
    m_textRenderingProgram.compileShaders("Shaders/textRenderingVert.txt", "Shaders/textRenderingFrag.txt");
    m_textRenderingProgram.addAttribute("vertexPosition");
    m_textRenderingProgram.addAttribute("vertexColor");
    m_textRenderingProgram.addAttribute("vertexUV");
    m_textRenderingProgram.linkShaders();

    // Init camera
    m_camera.init(m_window->getScreenWidth(), m_window->getScreenHeight());
    m_camera.setScale(20.0f); // 20.0f
    m_player = Player(&m_camera, m_blockManager);

    // Init player
    Bengine::ColorRGBA8 textureColor;
    textureColor.r = 255;
    textureColor.g = 255;
    textureColor.b = 255;
    textureColor.a = 255;

    m_player.init(&m_world, playerPos, glm::vec2(1.3f, 2.75f), textureColor, &m_camera);

}

void GameplayScreen::onExit() {

}

void GameplayScreen::update() {
    PROFILE_SCOPE("GameplayScreen::update");

    {
        PROFILE_SCOPE("camera.update");
        m_camera.update();
    }

    {
        PROFILE_SCOPE("GameplayScreen::checkInput");
        checkInput();
    }

    {
        PROFILE_SCOPE("GameplayScreen::b2World_Step");
        //Update the physics simulation
        float timeStep = 1.0f / 60.0f;
        int subStepCount = 4;
        b2World_Step(m_world, timeStep, subStepCount);
    }

    {
        PROFILE_SCOPE("player.update + camera.setPosition");
        const glm::vec2 playerPos = glm::vec2(b2Body_GetPosition(m_player.getID()).x, b2Body_GetPosition(m_player.getID()).y);
        m_player.update(m_game->inputManager, playerPos, m_blockManager);

        {
            PROFILE_SCOPE("Unload far chunks");
            m_blockManager->unloadFarChunks(playerPos);
        }
        {
            PROFILE_SCOPE("Load nearby chunks");
            m_blockManager->loadNearbyChunks(playerPos);
        }
        {
            PROFILE_SCOPE("BlockManager Update");
            m_blockManager->update(*m_blockManager);
        }


        m_camera.setPosition(playerPos); // Set camera position to player's position
    }


}

void GameplayScreen::draw() {
    PROFILE_SCOPE("GameplayScreen::draw");
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(0.4f, 0.4f, 0.8f, 1.0f);

    m_textureProgram.use();

    // Upload texture uniform
    GLint textureUniform = m_textureProgram.getUniformLocation("mySampler");
    glUniform1i(textureUniform, 0);
    glActiveTexture(GL_TEXTURE0);

    // Camera matrix
    glm::mat4 projectionMatrix = m_camera.getCameraMatrix();
    GLint pUniform = m_textureProgram.getUniformLocation("P");
    glUniformMatrix4fv(pUniform, 1, GL_FALSE, &projectionMatrix[0][0]);

    {
        PROFILE_SCOPE("Draw blocks");
        m_blockManager->renderBlocks();
    }
    m_spriteBatch.begin();
    {
        PROFILE_SCOPE("Draw player");
        m_player.draw(m_spriteBatch);
    }
    m_spriteBatch.end();
    m_spriteBatch.renderBatch();

    {
        PROFILE_SCOPE("drawImgui");
        Bengine::ImGuiManager::newFrame();

        drawImgui();

        Bengine::ImGuiManager::renderFrame();
    }
    m_textureProgram.unuse();
    m_textRenderingProgram.use();

    // Make sure the shader uses texture 0
    GLint textUniform = m_textRenderingProgram.getUniformLocation("mySampler");
    glUniform1i(textUniform, 0);

    GLint pUniform2 = m_textRenderingProgram.getUniformLocation("P");
    glUniformMatrix4fv(pUniform2, 1, GL_FALSE, &projectionMatrix[0][0]);

    drawHud();

    m_textRenderingProgram.unuse();

    if (m_debugRenderEnabled) {
        // Enable blending for transparency
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // Set debug drawing alpha
        DebugDraw::getInstance().setAlpha(m_debugAlpha);

        // Draw debug info
        DebugDraw::getInstance().drawWorld(&m_world, m_camera.getCameraMatrix());

    }

}

void GameplayScreen::checkInput() {
    SDL_Event evnt;
    while (SDL_PollEvent(&evnt)) {
        Bengine::ImGuiManager::processEvent(evnt);
        m_game->onSDLEvent(evnt);
    }
}

void GameplayScreen::drawHud() {
    const Bengine::ColorRGBA8 fontColor(255, 0, 0, 255);
    // Convert float to char *
    float _fps = 60.0f;
    char buffer[64];
    sprintf_s(buffer, "%.1f", _fps);
    m_spriteFont->draw(m_spriteBatch, buffer, glm::vec2(0.0f, m_window->getScreenHeight() - 32.0f),
        glm::vec2(1.0f), 0.0f, fontColor);
}

void GameplayScreen::drawImgui() {
    ImGui::Begin("Settings");

    static bool check = false;
    if (ImGui::Checkbox("Debug Renderer", &check)) {
        m_debugRenderEnabled = !m_debugRenderEnabled;
    }

    if (ImGui::InputFloat("Gravity", &m_gravity, 1.0f, 1.0f, "%.3f")) {
        updateGravity();
    }

    float jumpForce = m_player.getJumpForce(); // Get the current jump force
    if (ImGui::InputFloat("Jump Force", &jumpForce, 10.0f, 100.0f, "%.1f")) {
        m_player.setJumpForce(jumpForce); // Update the player's jump force
    }

    for (auto& result : m_profileResults)
    {
        char label[50];
        strcpy_s(label, result.Name);
        strcat_s(label, "  %.3fms");
        ImGui::Text(label, result.Time);
    }
    m_profileResults.clear();

    ImGui::End();

    // Separate debug window
    ImGui::SetNextWindowPos(ImVec2(220, 10), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(200, 150), ImGuiCond_FirstUseEver);
    ImGui::Begin("OpenGL Debug", nullptr, ImGuiWindowFlags_NoCollapse);

    ImGui::Text("OpenGL Errors: %d", Bengine::DebugOpenGL::GetErrorCount());

    if (ImGui::Button("Test Invalid Texture")) {
        glBindTexture(GL_TEXTURE_2D, 99999);
    }

    if (ImGui::Button("Test Invalid Enum")) {
        glEnable(GL_INVALID_ENUM);
    }

    if (ImGui::Button("Test Invalid Draw")) {
        glDrawArrays(GL_TRIANGLES, 0, -1);
    }

    if (ImGui::Button("Reset Error Count")) {
        Bengine::DebugOpenGL::ResetErrorCount();
    }

    ImGui::End();
}

void GameplayScreen::updateGravity() {
    b2Vec2 newGravity(0.0f, m_gravity);
    b2World_SetGravity(m_world, newGravity);
}
