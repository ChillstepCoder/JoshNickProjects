#include "GameplayScreen.h"
#include <iostream>
#include <SDL/SDL.h>
#include <Bengine/IMainGame.h>
#include <Bengine/ResourceManager.h>
#include "Bengine/ImGuiManager.h"
#include "PerlinNoise.hpp"
#include "Timer.h"

#define PROFILE_SCOPE(name) Timer timer##__LINE__(name,[&](ProfileResult profileResult) { m_profileResults.push_back(profileResult); })

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
    b2WorldDef worldDef = b2DefaultWorldDef();
    worldDef.gravity = b2Vec2(0.0f, m_gravity);
    m_world = b2CreateWorld(&worldDef);

    // Initialize BlockMeshManager
    m_blockMeshManager.init();

    m_blockManager = new BlockManager(m_blockMeshManager, m_world);

    // Create blocks using perlin noise
    generateWorld();

    m_blockManager->rebuildMesh();

    m_spriteBatch.init();


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
    m_camera.setScale(20.0f);
    m_player = Player(&m_camera, m_blockManager);

    Bengine::ColorRGBA8 textureColor;
    textureColor.r = 255;
    textureColor.g = 255;
    textureColor.b = 255;
    textureColor.a = 255;

    // Init player
    m_player.init(&m_world, glm::vec2(0.0f, 60.0f), glm::vec2(1.3f, 2.75f), textureColor, &m_camera);
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
        PROFILE_SCOPE("player.update");
        m_player.update(m_game->inputManager, m_blockManager->getBlocks());
    }

    {
        PROFILE_SCOPE("camera.setPosition");
        const glm::vec2 playerPos = glm::vec2(b2Body_GetPosition(m_player.getID()).x, b2Body_GetPosition(m_player.getID()).y);
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
}

void GameplayScreen::updateGravity() {
    b2Vec2 newGravity(0.0f, m_gravity);
    b2World_SetGravity(m_world, newGravity);
}

void GameplayScreen::generateWorld() {
    // Create Perlin noise instance with random seed
    siv::PerlinNoise perlin(12345); // Can change this seed for different terrain

    // Parameters for terrain generation
#ifdef _DEBUG
    // MAKE A WAY SMALLER WORLD IN DEBUG MODE SO ITS FASTER TO LOAD
    const int NUM_BLOCKS_X = 250;  // Width of the terrain
#else
    const int NUM_BLOCKS_X = 1500;  // Width of the terrain
#endif
    const float BLOCK_WIDTH = 1.0f;
    const float BLOCK_HEIGHT = 1.0f;
    const float START_X = -NUM_BLOCKS_X / 2;
    // Parameters for noise
    const float NOISE_SCALE = 0.05f;  // Controls how stretched the noise is
    const float AMPLITUDE = 10.0f;    // Controls the height variation
    const float SURFACE_Y = 14.0f;    // Base height of the surface

    Bengine::ColorRGBA8 textureColor;
    textureColor.r = 255;
    textureColor.g = 255;
    textureColor.b = 255;
    textureColor.a = 255;

    // Generate surface terrain
    std::vector<int> heightMap(NUM_BLOCKS_X);
    for (int x = 0; x < NUM_BLOCKS_X; x++) {
        // Generate height using Perlin noise
        float noiseValue = perlin.noise1D(x * NOISE_SCALE);
        int height = static_cast<int>(SURFACE_Y + noiseValue * AMPLITUDE);
        heightMap[x] = height;

        // Create surface (grass) blocks
        float worldX = START_X + x * BLOCK_WIDTH;
        Block surfaceBlock;
        glm::vec2 position(worldX, height * BLOCK_HEIGHT);
        m_texture = Bengine::ResourceManager::getTexture("Textures/connectedGrassBlock.png");
        surfaceBlock.init(&m_world, position, glm::vec2(BLOCK_WIDTH, BLOCK_HEIGHT),
            m_texture, textureColor, false);
        m_blockManager->addBlock(surfaceBlock);

        // Fill blocks below surface with dirt
        m_texture = Bengine::ResourceManager::getTexture("Textures/connectedDirtBlock.png");
        for (int y = height - 1; y > height - 10; y--) {
            Block dirtBlock;
            glm::vec2 dirtPos(worldX, y * BLOCK_HEIGHT);
            dirtBlock.init(&m_world, dirtPos, glm::vec2(BLOCK_WIDTH, BLOCK_HEIGHT),
                m_texture, textureColor, false);
            m_blockManager->addBlock(dirtBlock);
        }

        // Fill deeper blocks with stone
        m_texture = Bengine::ResourceManager::getTexture("Textures/connectedStoneBlock.png");
        for (int y = height - 10; y > height - 50; y--) {
            Block stoneBlock;
            glm::vec2 stonePos(worldX, y * BLOCK_HEIGHT);
            stoneBlock.init(&m_world, stonePos, glm::vec2(BLOCK_WIDTH, BLOCK_HEIGHT),
                m_texture, textureColor, false);
            m_blockManager->addBlock(stoneBlock);
        }
    }

    // Generate caves using 2D Perlin noise
    const float CAVE_SCALE = 0.4f;
    const float CAVE_THRESHOLD = 0.4f; // Adjust this to control cave density

    for (int x = 0; x < NUM_BLOCKS_X; x++) {
        for (int y = heightMap[x] - 8; y > heightMap[x] - 35; y--) {
            float caveNoise = perlin.noise2D(x * CAVE_SCALE, y * CAVE_SCALE);
            if (caveNoise > CAVE_THRESHOLD) {
                // Find and remove blocks at this position
                float worldX = START_X + x * BLOCK_WIDTH;
                float worldY = y * BLOCK_HEIGHT;

                auto it = std::remove_if(m_blockManager->getBlocks().begin(), m_blockManager->getBlocks().end(),
                    [worldX, worldY, BLOCK_WIDTH, BLOCK_HEIGHT](Block& block) {
                        b2Vec2 pos = block.getPosition();
                        return (pos.x >= worldX - BLOCK_WIDTH / 2 &&
                            pos.x <= worldX + BLOCK_WIDTH / 2 &&
                            pos.y >= worldY - BLOCK_HEIGHT / 2 &&
                            pos.y <= worldY + BLOCK_HEIGHT / 2);
                    });

                m_blockManager->getBlocks().erase(it, m_blockManager->getBlocks().end());
            }
        }
    }

    // Load the copper ore texture
    m_texture = Bengine::ResourceManager::getTexture("Textures/connectedCopperBlock.png");

    // Parameters for ore vein generation
    const float ORE_VEIN_SCALE = 0.03f;  // Controls how stretched the ore veins are
    const float ORE_VEIN_THRESHOLD = 0.25f; // Adjust this to control ore vein density
    const int MIN_VEIN_LENGTH = 5;      // Minimum length of an ore vein
    const int MAX_VEIN_LENGTH = 9;     // Maximum length of an ore vein

    // Generate copper ore veins
    for (int x = 0; x < NUM_BLOCKS_X; x++) {
        for (int y = heightMap[x] - 12; y > heightMap[x] - 35; y--) {
            float oreNoise = perlin.noise2D(x * ORE_VEIN_SCALE, y * ORE_VEIN_SCALE);
            if (oreNoise > ORE_VEIN_THRESHOLD) {
                // Start a new ore vein
                int veinLength = MIN_VEIN_LENGTH + rand() % (MAX_VEIN_LENGTH - MIN_VEIN_LENGTH + 1);
                int veinStartX = x;
                int veinStartY = y;

                for (int i = 0; i < veinLength; i++) {
                    // Create ore blocks along the vein
                    float worldX = START_X + veinStartX * BLOCK_WIDTH;
                    float worldY = veinStartY * BLOCK_HEIGHT;
                    Block oreBlock;
                    oreBlock.init(&m_world, glm::vec2(worldX, worldY), glm::vec2(BLOCK_WIDTH, BLOCK_HEIGHT),
                        m_texture, textureColor, false);
                    m_blockManager->addBlock(oreBlock);

                    // Move to the next position in the vein
                    veinStartX++;
                    if (veinStartX >= NUM_BLOCKS_X) {
                        // Wrap around to the beginning if we reach the end
                        veinStartX = 0;
                    }
                }
            }
        }
    }

    m_texture = Bengine::ResourceManager::getTexture("Textures/connectedIronBlock.png");

    // Parameters for iron vein generation
    const float ORE_VEIN_SCALE2 = 0.02f;  // Controls how stretched the ore veins are
    const float ORE_VEIN_THRESHOLD2 = 0.45f; // Adjust this to control ore vein density
    const int MIN_VEIN_LENGTH2 = 5;      // Minimum length of an ore vein
    const int MAX_VEIN_LENGTH2 = 9;     // Maximum length of an ore vein

    // Generate iron ore veins
    for (int x = 0; x < NUM_BLOCKS_X; x++) {
        for (int y = heightMap[x] - 12; y > heightMap[x] - 35; y--) {
            float oreNoise = perlin.noise2D(x * ORE_VEIN_SCALE2, y * ORE_VEIN_SCALE2);
            if (oreNoise > ORE_VEIN_THRESHOLD2) {
                // Start a new ore vein
                int veinLength = MIN_VEIN_LENGTH2 + rand() % (MAX_VEIN_LENGTH2 - MIN_VEIN_LENGTH2 + 1);
                int veinStartX = x;
                int veinStartY = y;

                for (int i = 0; i < veinLength; i++) {
                    // Create ore blocks along the vein
                    float worldX = START_X + veinStartX * BLOCK_WIDTH;
                    float worldY = veinStartY * BLOCK_HEIGHT;
                    Block oreBlock;
                    oreBlock.init(&m_world, glm::vec2(worldX, worldY), glm::vec2(BLOCK_WIDTH, BLOCK_HEIGHT),
                        m_texture, textureColor, false);
                    m_blockManager->addBlock(oreBlock);

                    // Move to the next position in the vein
                    veinStartX++;
                    if (veinStartX >= NUM_BLOCKS_X) {
                        // Wrap around to the beginning if we reach the end
                        veinStartX = 0;
                    }
                }
            }
        }
    }
}