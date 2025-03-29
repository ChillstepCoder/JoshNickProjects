#include "GameplayScreen.h"
#include <iostream>
#include <SDL/SDL.h>
#include <Bengine/IMainGame.h>
#include <Bengine/ResourceManager.h>
#include "Bengine/ImGuiManager.h"
#include "PerlinNoise.hpp"
#include "Timer.h"
#include "Profiler.h"
#include "ConnectedTextureSet.h"


GameplayScreen::GameplayScreen(Bengine::Window* window) : m_window(window), m_connectedTextureSet(ConnectedTextureSet::getInstance()) {

}
GameplayScreen::~GameplayScreen() {
    delete m_blockManager;
    b2DestroyWorld(m_world);
}

int GameplayScreen::getNextScreenIndex() const {
    return m_screenIndex + 1;;
}

int GameplayScreen::getPreviousScreenIndex() const {
    return m_screenIndex - 1;
}

void GameplayScreen::build() {

}

void GameplayScreen::destroy() {

}

void GameplayScreen::onEntry() {
    glm::vec2 playerPos(1024.0f, 1680.0f);

    b2WorldDef worldDef = b2DefaultWorldDef();
    worldDef.gravity = b2Vec2(0.0f, m_gravity);
    m_world = b2CreateWorld(&worldDef);

    BlockDefRepository::initBlockDefs();

    m_spriteBatch.init();

    // Initialize BlockMeshManager
    m_blockMeshManager.init();

    m_cellularAutomataManager.init();

    m_connectedTextureSet.LoadRules();

    m_blockManager = new BlockManager(m_blockMeshManager, m_world, m_cellularAutomataManager);

    m_blockManager->initializeChunks(playerPos);

    DebugDraw::getInstance().init();

    DebugDraw::getInstance().setAlpha(m_debugAlpha);

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

    // Set map Bounds
    glm::vec2 minBounds(0.0f, 0.0f);
    glm::vec2 maxBounds(WORLD_WIDTH_CHUNKS * CHUNK_WIDTH, WORLD_HEIGHT_CHUNKS * CHUNK_WIDTH);

    setMapBoundaries(minBounds, maxBounds);

    m_blockManager->loadNearbyChunks(playerPos, *m_blockManager, m_lightingSystem);

    m_lightingSystem.init(WORLD_WIDTH_CHUNKS * CHUNK_WIDTH, WORLD_HEIGHT_CHUNKS * CHUNK_WIDTH);

    m_blockManager->setLightingSystem(&m_lightingSystem);

    m_lightingSystem.setBlockManager(m_blockManager);

    {
        PROFILE_SCOPE("Initial Lighting Update");
        m_lightingSystem.updateLighting();
    }


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
        // Get current player position
        glm::vec2 playerPos = m_player.getPosition();

        // Clamp player position to map boundaries
        playerPos.x = std::max(m_mapMinBounds.x, std::min(playerPos.x, m_mapMaxBounds.x));
        playerPos.y = std::max(m_mapMinBounds.y, std::min(playerPos.y, m_mapMaxBounds.y));

        // Set player position (if it has a setPosition method)
        m_player.setPosition(playerPos);

        // Now update player with the clamped position
        m_player.update(m_game->inputManager, playerPos, m_blockManager, m_debugRenderEnabled, m_lightingSystem);

        {
            PROFILE_SCOPE("Unload far chunks");
            m_blockManager->unloadFarChunks(playerPos, m_lightingSystem);
        }
        {
            PROFILE_SCOPE("Load nearby chunks");
            if (m_blockManager->loadNearbyChunks(playerPos, *m_blockManager, m_lightingSystem)) {
                std::vector<Chunk*> newlyLoadedChunks = m_blockManager->getNewlyLoadedChunks();

                // Update lighting for each newly loaded chunk
                for (auto* chunk : newlyLoadedChunks) {
                    // Calculate the world position of the chunk
                    int chunkX = static_cast<int>(chunk->getWorldPosition().x) / CHUNK_WIDTH;
                    int chunkY = static_cast<int>(chunk->getWorldPosition().y) / CHUNK_WIDTH;

                    // Update lighting for this chunk and adjacent chunks
                    m_lightingSystem.updateLightingForRegion(chunkX, chunkY, *m_blockManager);
                }
            }
        }

        {
            PROFILE_SCOPE("BlockManager Update");
            if (m_updateFrame % 5 == 0)
                m_blockManager->update(*m_blockManager, m_lightingSystem);
        }


        m_camera.setPosition(playerPos); // Set camera position to player's position
    }

    m_updateFrame++;
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

    m_spriteBatch.begin();
    {
        PROFILE_SCOPE("Draw player");
        m_player.draw(m_spriteBatch);
    }
    m_spriteBatch.end();
    m_spriteBatch.renderBatch();
    {
        PROFILE_SCOPE("Draw blocks");
        m_blockManager->renderBlocks();
    }

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

    if (m_player.getScreenIndex() == 0) {
        m_game->getCurrentScreen()->setState(Bengine::ScreenState::CHANGE_PREVIOUS);
    }

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

    // Sliders for world generation parameters
    ImGui::SliderFloat("Cave Scale", &m_caveScale, 0.001f, 0.1f, "%.5f");
    ImGui::SliderFloat("Base Cave Threshold", &m_baseCaveThreshold, 0.0f, 1.0f, "%.2f");
    ImGui::SliderFloat("Detail Scale", &m_detailScale, 0.001f, 0.1f, "%.5f");
    ImGui::SliderFloat("Detail Influence", &m_detailInfluence, 0.0f, 1.0f, "%.2f");
    ImGui::SliderFloat("Min Cave Depth", &m_minCaveDepth, 0.0f, 100.0f, "%.1f");
    ImGui::SliderFloat("Surface Zone", &m_surfaceZone, 0.0f, 200.0f, "%.1f");
    ImGui::SliderFloat("Deep Zone", &m_deepZone, 0.0f, 1000.0f, "%.1f");
    ImGui::SliderFloat("Max Surface Bonus", &m_maxSurfaceBonus, 0.0f, 0.1f, "%.4f");
    ImGui::SliderFloat("Max Depth Penalty", &m_maxDepthPenalty, 0.0f, 0.1f, "%.4f");

    if (ImGui::Button("Regenerate World")) {
        m_blockManager->regenerateWorld(m_caveScale, m_baseCaveThreshold, m_detailScale, m_detailInfluence, m_minCaveDepth, m_surfaceZone, m_deepZone, m_maxSurfaceBonus, m_maxDepthPenalty);
    }

    const auto& latestResults = Profiler::Get().GetLatestResults();
    const auto& maxTimes = Profiler::Get().GetMaxTimes();

    for (const auto& [name, result] : latestResults)
    {
        ImGui::Text("%s  %.3fms (Max: %.3fms)",
            name.c_str(), result.Time, maxTimes.at(name));
    }

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
